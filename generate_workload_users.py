import random
import sys

def generate_workload(num_users=10, num_user_attrs=20, num_rules=500, table_name="my_table"):
    # --- Generate dynamic user list ---
    users = [f"user{i}" for i in range(1, num_users + 1)]

    sql = []
    sql.append("SET ENV_ATTRIBUTE workday = monday, tuesday, wednesday, thursday, friday, saturday, sunday;")

    # --- Create users ---
    sql.append("\n-- Create users")
    for u in users:
        sql.append(f"CREATE USER {u};")

    # --- Create table ---
    sql.append(f"""
-- Create table
CREATE TABLE {table_name} (
    id SERIAL PRIMARY KEY,
    name TEXT,
    privacy TEXT,
    category TEXT
);

-- Insert sample data
INSERT INTO {table_name} (name, privacy, category) VALUES
('record1', 'level1', 'finance'),
('record2', 'level2', 'hr'),
('record3', 'level3', 'engg'),
('record4', 'level1', 'finance'),
('record5', 'level2', 'hr');
""")

    # --- Define user attributes ---
    sql.append("\n-- Define user attributes")
    for i in range(num_user_attrs):
        sql.append(f"CREATE USER_ATTRIBUTE attr{i};")

    # --- Grant random user attributes to users ---
    sql.append("\n-- Grant user attributes")
    values = ["low", "medium", "high", "kgp", "blr", "cs", "mech", "hr", "finance", "admin", "student", "faculty"]
    for u in users:
        for i in range(num_user_attrs):
            val = random.choice(values)
            sql.append(f"GRANT USER_ATTRIBUTE {{attr{i}={val}}} TO {u};")

    # --- Define resource attributes ---
    sql.append("\n-- Define resource attributes")
    sql.append("CREATE RESOURCE_ATTRIBUTE privacy;")
    sql.append("CREATE RESOURCE_ATTRIBUTE category;")
    sql.append(f"""
GRANT RESOURCE_ATTRIBUTE {{privacy=level1}} TO {table_name};
GRANT RESOURCE_ATTRIBUTE {{category=finance}} TO {table_name};
GRANT RESOURCE_ATTRIBUTE {{privacy=level2}} TO {table_name};
GRANT RESOURCE_ATTRIBUTE {{category=hr}} TO {table_name};
GRANT RESOURCE_ATTRIBUTE {{privacy=level3}} TO {table_name};
GRANT RESOURCE_ATTRIBUTE {{category=engg}} TO {table_name};
""")

    # --- Create ABAC rules ---
    sql.append("\n-- Define ABAC rules")
    for i in range(num_rules):
        num_user_attrs_in_rule = random.randint(1, min(3, num_user_attrs))
        attrs_in_rule = random.sample(range(num_user_attrs), num_user_attrs_in_rule)
        user_clause = ", ".join([f"attr{a}={random.choice(values)}" for a in attrs_in_rule])

        privacy = random.choice(["level1", "level2", "level3"])
        category = random.choice(["finance", "hr", "engg"])

        sql.append(f"CREATE ABAC_RULE rule{i} FOR SELECT OF USER_ATTRIBUTE ({user_clause}) "
                   f"RESOURCE_ATTRIBUTE (privacy={privacy}, category={category}) ENV_ATTRIBUTE true;")

    # --- Add access test section ---
    sql.append("\n-- Test access for each user")
    u = users[0]
    sql.append(f"""
\\echo '\\n--- {u} ---'
SET SESSION AUTHORIZATION {u};
\\timing on
SELECT * FROM {table_name};
\\timing off
""")

    sql.append("RESET SESSION AUTHORIZATION;")

    # --- Clean slate ---
    sql.append("-- Clean slate")
    sql.append(f"DROP TABLE IF EXISTS {table_name} CASCADE;")
    sql.append("DROP USER IF EXISTS " + ", ".join(users) + ";")
    for i in range(num_rules):
        sql.append(f"DROP ABAC_RULE rule{i};")

    for i in range(num_user_attrs):
        sql.append(f"DROP USER_ATTRIBUTE attr{i};")

    sql.append("DROP RESOURCE_ATTRIBUTE privacy;")
    sql.append("DROP RESOURCE_ATTRIBUTE category;")

    return "\n".join(sql)


if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python generate_workload_users.py <num_users> <num_user_attributes> <num_rules>")
        sys.exit(1)

    num_users = int(sys.argv[1])
    num_user_attrs = int(sys.argv[2])
    num_rules = int(sys.argv[3])

    script = generate_workload(num_users, num_user_attrs, num_rules)
    with open("generated_workload.sql", "w") as f:
        f.write(script)

    print(f"Generated workload with {num_users} users, {num_user_attrs} user attributes, and {num_rules} ABAC rules in 'generated_workload.sql'")
