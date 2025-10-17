import random
import sys

def generate_workload(num_res_attrs=2, num_rules=10, num_user_attrs=2, user="alice"):
    sql = []

    table_names = [f"table{i}" for i in range(1, 6)]

    # --- Clean slate ---
    sql.append("-- Clean slate")
    for t in table_names:
        sql.append(f"DROP TABLE IF EXISTS {t} CASCADE;")
    sql.append(f"DROP USER IF EXISTS {user};")
    for i in range(num_rules):
        sql.append(f"DROP ABAC_RULE rule{i};")
    for i in range(num_user_attrs):
        sql.append(f"DROP USER_ATTRIBUTE attr{i};")
    for i in range(num_res_attrs):
        sql.append(f"DROP RESOURCE_ATTRIBUTE resattr{i};")

    # --- Create user ---
    sql.append("\n-- Create user")
    sql.append(f"CREATE USER {user};")

    # --- Create tables ---
    sql.append("\n-- Create tables and insert data")
    for t in table_names:
        sql.append(f"""
CREATE TABLE {t} (
    id SERIAL PRIMARY KEY,
    name TEXT
);
INSERT INTO {t} (name) VALUES
('record1'), ('record2'), ('record3'), ('record4'), ('record5');
""")

    # --- Define user attributes ---
    sql.append("\n-- Define user attributes")
    for i in range(num_user_attrs):
        sql.append(f"CREATE USER_ATTRIBUTE attr{i};")

    # --- Grant random user attributes ---
    sql.append("\n-- Grant user attributes")
    values = ["low", "medium", "high", "kgp", "blr", "cs", "mech", "hr", "finance", "admin", "student", "faculty"]
    for i in range(num_user_attrs):
        val = random.choice(values)
        sql.append(f"GRANT USER_ATTRIBUTE {{attr{i}={val}}} TO {user};")

    # --- Define resource attributes ---
    sql.append("\n-- Define resource attributes")
    for i in range(num_res_attrs):
        sql.append(f"CREATE RESOURCE_ATTRIBUTE resattr{i};")

    # --- Grant random resource attributes to tables ---
    sql.append("\n-- Grant resource attributes to tables")
    for t in table_names:
        for i in range(num_res_attrs):
            val = random.choice(values)
            sql.append(f"GRANT RESOURCE_ATTRIBUTE {{resattr{i}={val}}} TO {t};")

    # --- Create ABAC rules ---
    sql.append("\n-- Define ABAC rules")
    for i in range(num_rules):
        num_user_attrs_in_rule = random.randint(1, min(3, num_user_attrs))
        attrs_in_rule = random.sample(range(num_user_attrs), num_user_attrs_in_rule)
        user_clause = ", ".join([f"attr{a}={random.choice(values)}" for a in attrs_in_rule])

        num_res_attrs_in_rule = random.randint(1, min(2, num_res_attrs))
        res_attrs_in_rule = random.sample(range(num_res_attrs), num_res_attrs_in_rule)
        res_clause = ", ".join([f"resattr{r}={random.choice(values)}" for r in res_attrs_in_rule])

        sql.append(f"CREATE ABAC_RULE rule{i} FOR SELECT OF USER_ATTRIBUTE ({user_clause}) "
                   f"RESOURCE_ATTRIBUTE ({res_clause}) ENV_ATTRIBUTE true;")

    # --- Access test section ---
    sql.append("\n-- Test access for user on all tables")
    sql.append(f"""
\\echo '\\n--- {user.capitalize()} ---'
SET SESSION AUTHORIZATION {user};
\\timing on
""")

    for t in table_names:
        sql.append(f"\\echo '\\nAccessing {t}'")
        sql.append(f"SELECT * FROM {t};")

    sql.append("\\timing off")
    sql.append("RESET SESSION AUTHORIZATION;")

    return "\n".join(sql)


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python generate_workload.py <num_resource_attributes> <num_abac_rules>")
        sys.exit(1)

    num_res_attrs = int(sys.argv[1])
    num_rules = int(sys.argv[2])

    script = generate_workload(num_res_attrs, num_rules)
    with open("generated_workload.sql", "w") as f:
        f.write(script)

    print(f"Generated workload with {num_res_attrs} resource attrs, "
          f"and {num_rules} ABAC rules for 1 user accessing 5 tables in 'generated_workload.sql'")
