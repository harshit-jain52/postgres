import time
import random
import string
from tqdm import trange
from statistics import mean
import psycopg2
from psycopg2 import sql
from dotenv import dotenv_values

# -------------------------------------------------------
# DB CONNECTION
# -------------------------------------------------------

env_vars = dotenv_values(".env")

def get_conn():
    return psycopg2.connect(
        host=env_vars["PGHOST"],
        port=env_vars["PGPORT"],
        dbname=env_vars["PGDATABASE"],
        user=env_vars["PGUSER"],
        password=env_vars["PGPASSWORD"],
    )


# -------------------------------------------------------
# HELPERS
# -------------------------------------------------------

def rand_name(prefix, n=6):
    return prefix + "_" + "".join(random.choices(string.ascii_lowercase, k=n))


def exec_many(cur, statements):
    for stmt in statements:
        cur.execute(stmt)


# -------------------------------------------------------
# CLEANUP
# -------------------------------------------------------

def cleanup(cur):
    """
    Drops all users, tables, attributes, and ABAC rules created by the benchmark.
    """
    cur.execute("""
        DO $$
        DECLARE r RECORD;
        BEGIN
            -- Drop tables
            FOR r IN SELECT tablename FROM pg_tables WHERE tablename LIKE 't_%'
            LOOP
                EXECUTE 'DROP TABLE IF EXISTS ' || r.tablename || ' CASCADE';
            END LOOP;

            -- Drop users
            FOR r IN SELECT rolname FROM pg_roles WHERE rolname LIKE 'u_%'
            LOOP
                EXECUTE 'DROP ROLE IF EXISTS ' || r.rolname;
            END LOOP;

            -- Drop ABAC rules
            FOR r IN SELECT rulename FROM pg_abac_rule_priv
            LOOP
                EXECUTE 'DROP ABAC_RULE ' || r.rulename;
            END LOOP;

            -- Drop attributes
            FOR r IN SELECT attrib_name FROM pg_user_attr
            LOOP
                EXECUTE 'DROP USER_ATTRIBUTE ' || r.attrib_name;
            END LOOP;

            FOR r IN SELECT attrib_name FROM pg_resource_attr
            LOOP
                EXECUTE 'DROP RESOURCE_ATTRIBUTE ' || r.attrib_name;
            END LOOP;
        END $$;
    """)


# -------------------------------------------------------
# CORE EXPERIMENT
# -------------------------------------------------------

def run_experiment(
    num_abac_rules: int,
    num_user_attrs: int,
    num_resource_attrs: int,
    num_users: int,
    num_tables: int,
):
    """
    Generates schema + ABAC config and measures average SELECT time.
    """

    conn = get_conn()
    conn.autocommit = True
    cur = conn.cursor()

    # ---------- CREATE USERS ----------
    users = [f"u_{i}" for i in range(num_users)]
    for u in users:
        cur.execute(sql.SQL("CREATE ROLE {} LOGIN").format(sql.Identifier(u)))

    # ---------- CREATE TABLES ----------
    tables = [f"t_{i}" for i in range(num_tables)]
    for t in tables:
        cur.execute(sql.SQL("""
            CREATE TABLE {} (
                id INT PRIMARY KEY,
                val TEXT
            )
        """).format(sql.Identifier(t)))

        cur.execute(sql.SQL("""
            INSERT INTO {} VALUES (1, 'hello')
        """).format(sql.Identifier(t)))

        # DAC baseline: grant SELECT
        cur.execute(sql.SQL("GRANT SELECT ON {} TO PUBLIC").format(sql.Identifier(t)))

    # ---------- ATTRIBUTES ----------
    user_attrs = [f"ua_{i}" for i in range(num_user_attrs)]
    res_attrs = [f"ra_{i}" for i in range(num_resource_attrs)]

    for ua in user_attrs:
        cur.execute(f"CREATE USER_ATTRIBUTE {ua}")

    for ra in res_attrs:
        cur.execute(f"CREATE RESOURCE_ATTRIBUTE {ra}")

    # Assign attribute values
    for u in users:
        for ua in user_attrs:
            cur.execute(
                f"GRANT USER_ATTRIBUTE {{{ua} = v1}} TO {u}"
            )

    for t in tables:
        for ra in res_attrs:
            cur.execute(
                f"GRANT RESOURCE_ATTRIBUTE {{{ra} = v1}} TO TABLE {t}"
            )

    # ---------- ABAC RULES ----------
    if num_abac_rules > 0:
        for i in range(num_abac_rules):
            rule = f"r_{i}"

            ua_clause = ", ".join(f"{ua} = v1" for ua in user_attrs[:1])
            ra_clause = ", ".join(f"{ra} = v1" for ra in res_attrs[:1])

            cur.execute(f"""
                CREATE ABAC_RULE {rule}
                FOR SELECT OF
                USER_ATTRIBUTE ({ua_clause})
                RESOURCE_ATTRIBUTE ({ra_clause})
                ENV_ATTRIBUTE (workday=true)
            """)

    # ---------- MEASUREMENT ----------
    timings = []

    for u in users:
        user_conn = psycopg2.connect(
            host=env_vars["PGHOST"],
            port=env_vars["PGPORT"],
            dbname=env_vars["PGDATABASE"],
            user=u,
            password=env_vars["PGPASSWORD"],
        )
        user_conn.autocommit = True
        ucur = user_conn.cursor()

        for t in tables:
            start = time.perf_counter()
            ucur.execute(sql.SQL("SELECT * FROM {}").format(sql.Identifier(t)))
            ucur.fetchall()
            end = time.perf_counter()
            timings.append(end - start)

        user_conn.close()

    cur.close()
    conn.close()

    return mean(timings)


# -------------------------------------------------------
# MULTI-ITERATION DRIVER
# -------------------------------------------------------

def benchmark(
    num_iterations: int,
    **experiment_params
):
    results = []

    for _ in trange(num_iterations, desc="Benchmark iterations"):
        # print(f"[+] Iteration {i+1}/{num_iterations}")

        conn = get_conn()
        conn.autocommit = True
        cur = conn.cursor()

        cleanup(cur)
        cur.close()
        conn.close()

        avg_time = run_experiment(**experiment_params)
        results.append(avg_time)

        # print(f"    Avg SELECT time: {avg_time*1000:.3f} ms")

    final_avg = mean(results)
    print(f"Average over {num_iterations} runs: {final_avg*1000:.3f} ms")
    # print(**experiment_params)

    return final_avg


if __name__ == "__main__":
    for num_abac_rules in [10, 50, 100, 500, 1000]:
        for num_user_attrs in [10, 15, 20, 25, 30]:
            print(f"\n=== ABAC Rules: {num_abac_rules}, User Attrs: {num_user_attrs} ===")
            benchmark(
                num_iterations=100,
                num_abac_rules=num_abac_rules,
                num_user_attrs=num_user_attrs,
                num_resource_attrs=10,
                num_users=10,
                num_tables=10,
            )
    
    for num_abac_rules in [10, 50, 100, 500, 1000]:
        for num_users in [10, 15, 20, 25, 30]:
            print(f"\n=== ABAC Rules: {num_abac_rules}, Users: {num_users} ===")
            benchmark(
                num_iterations=100,
                num_abac_rules=num_abac_rules,        # 0 => PostgreSQL baseline (no ABAC)
                num_user_attrs=10,
                num_resource_attrs=10,
                num_users=num_users,
                num_tables=10,
            )

    for num_user_attrs in [10, 20, 50, 100]:
        for num_users in [10, 20, 50, 100]:
            print(f"\n=== User Attrs: {num_user_attrs}, Users: {num_users} ===")
            benchmark(
                num_iterations=100,
                num_abac_rules=10,
                num_user_attrs=num_user_attrs,
                num_resource_attrs=10,
                num_users=num_users,
                num_tables=10,
            )
