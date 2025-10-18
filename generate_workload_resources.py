import random
import sys

def generate_workload(num_tables=5, num_resource_attrs=10, num_rules=100, table_prefix="tbl"):
    user = "user1"
    num_user_attrs = 2  # fixed as per requirement

    sql = []
    values = ["low", "medium", "high", "finance", "hr", "engg", "admin", "student", "faculty", "kgp", "blr"]

    # --- ENV ATTRIBUTES ---
    sql.append("SET ENV_ATTRIBUTE workday = monday, tuesday, wednesday, thursday, friday, saturday, sunday;")

    # --- Create one user ---
    sql.append("\n-- Create user")
    sql.append(f"CREATE USER {user};")

    # --- Create multiple tables ---
    sql.append("\n-- Create tables")
    for i in range(1, num_tables + 1):
        table_name = f"{table_prefix}{i}"
        sql.append(f"""
CREATE TABLE {table_name} (
    id SERIAL PRIMARY KEY,
    name TEXT,
    privacy TEXT,
    category TEXT
);

INSERT INTO {table_name} (name, privacy, category) VALUES
('record1', 'level1', 'finance'),
('record2', 'level2', 'hr'),
('record3', 'level3', 'engg');
""")

    # --- Define user attributes (fixed 2) ---
    sql.append("\n-- Define user attributes")
    for i in range(num_user_attrs):
        sql.append(f"CREATE USER_ATTRIBUTE uattr{i};")

    # --- Grant random user attribute values ---
    sql.append("\n-- Grant user attributes")
    for i in range(num_user_attrs):
        val = random.choice(values)
        sql.append(f"GRANT USER_ATTRIBUTE {{uattr{i}={val}}} TO {user};")

    # --- Define resource attributes ---
    sql.append("\n-- Define resource attributes")
    for i in range(num_resource_attrs):
        sql.append(f"CREATE RESOURCE_ATTRIBUTE rattr{i};")

    # --- Grant random resource attributes to each table ---
    sql.append("\n-- Grant resource attributes to tables")
    for i in range(1, num_tables + 1):
        table_name = f"{table_prefix}{i}"
        for j in range(num_resource_attrs):
            val = random.choice(values)
            sql.append(f"GRANT RESOURCE_ATTRIBUTE {{rattr{j}={val}}} TO {table_name};")

    # --- Create ABAC rules ---
    sql.append("\n-- Define ABAC rules")
    for i in range(num_rules):
        num_user_attrs_in_rule = random.randint(1, num_user_attrs)
        user_clause = ", ".join([f"uattr{a}={random.choice(values)}" for a in range(num_user_attrs_in_rule)])

        num_res_attrs_in_rule = random.randint(1, min(3, num_resource_attrs))
        res_attrs = random.sample(range(num_resource_attrs), num_res_attrs_in_rule)
        res_clause = ", ".join([f"rattr{r}={random.choice(values)}" for r in res_attrs])

        sql.append(f"CREATE ABAC_RULE rule{i} FOR SELECT OF USER_ATTRIBUTE ({user_clause}) "
                   f"RESOURCE_ATTRIBUTE ({res_clause}) ENV_ATTRIBUTE true;")

    # --- Test access for the first table ---
    sql.append("\n-- Test access for user1 on the first table")
    table_name = f"{table_prefix}1"
    sql.append(f"""
\\echo '\\n--- {user} accessing {table_name} ---'
SET SESSION AUTHORIZATION {user};
\\timing on
SELECT * FROM {table_name};
\\timing off
RESET SESSION AUTHORIZATION;
""")

    # --- Clean slate ---
    sql.append("\n-- Clean slate")
    for i in range(num_rules):
        sql.append(f"DROP ABAC_RULE rule{i};")

    for i in range(num_resource_attrs):
        sql.append(f"DROP RESOURCE_ATTRIBUTE rattr{i};")

    for i in range(num_user_attrs):
        sql.append(f"DROP USER_ATTRIBUTE uattr{i};")

    sql.append(f"DROP USER IF EXISTS {user};")

    for i in range(1, num_tables + 1):
        sql.append(f"DROP TABLE IF EXISTS {table_prefix}{i} CASCADE;")

    return "\n".join(sql)


if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python generate_workload_resources.py <num_tables> <num_resource_attributes> <num_rules>")
        sys.exit(1)

    num_tables = int(sys.argv[1])
    num_resource_attrs = int(sys.argv[2])
    num_rules = int(sys.argv[3])

    script = generate_workload(num_tables, num_resource_attrs, num_rules)
    with open("generated_workload.sql", "w") as f:
        f.write(script)

    print(f"Generated workload with {num_tables} tables, {num_resource_attrs} resource attributes, and {num_rules} ABAC rules in 'generated_workload.sql'")
