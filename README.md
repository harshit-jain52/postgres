ASPS: ABAC-Supported PostgreSQL
================================

This directory contains the source code distribution of ASPS — a modified version of the PostgreSQL database management system with native support for Attribute-Based Access Control (ABAC).

ASPS extends PostgreSQL’s access control framework by integrating attribute-driven policies alongside traditional Discretionary Access Control (DAC) and Role-Based Access Control (RBAC). It introduces new SQL syntax, system catalogs, and enforcement mechanisms for managing user, resource, and environmental attributes. In addition to supporting standard PostgreSQL features such as transactions, triggers, stored procedures, and user-defined functions, ASPS allows defining and enforcing ABAC rules directly within the database engine.

Copyright and license information can be found in the file COPYRIGHT.

## ABAC-specific SQL Statements

```sql
CREATE USER_ATTRIBUTE name;
CREATE RESOURCE_ATTRIBUTE name;
DROP USER_ATTRIBUTE name;
DROP RESOURCE_ATTRIBUTE name;

GRANT USER_ATTRIBUTE { name = val }
    TO user1, user2 ...;
GRANT RESOURCE_ATTRIBUTE { name = val }
    TO dbobj1, dbobj2 ...;
REVOKE USER_ATTRIBUTE { name = val|ALL }
    FROM user1, user2 ...;
REVOKE RESOURCE_ATTRIBUTE { name = val|ALL }
    FROM dbobj1, dbobj2 ...;

SET ENV_ATTRIBUTE workday = monday, tuesday ...;

CREATE ABAC_RULE name 
    FOR SELECT, UPDATE ... OF
    USER_ATTRIBUTE ( attr1=val1|ANY, ... ) 
    RESOURCE_ATTRIBUTE ( attr2=val2|ANY, ... )
    ENV_ATTRIBUTE is_workday;
DROP ABAC_RULE name;
```

## Files Modified/Created

### 1. Grammar Augmentation

- `src/backend/parser/gram.y`: Definition of SQL grammar rules
- `src/include/parser/kwlist.h`: List of SQL keywords
- `src/include/tcop/cmdtaglist.h`: List of SQL command tags
- `src/include/nodes/parsenodes.h`: Data structure type for nodes of the parse tree
- `src/backend/tcop/utility.c`: Mapping of parse tree nodes to handler functions
- `src/backend/commands/abac.c`: Handler functions corresponding to SQL statements
- `src/backend/utils/adt/acl.c`: Helper functions for the handler functions
- `src/include/commands/abac.h`: Header file for `abac.c`
- `src/backend/commands/meson.build`: Build file for `abac.c`
- `src/backend/commands/Makefile`: Makefile for `abac.c`

### 2. System Catalogs

- `src/include/catalog/pg_user_attr.h`: Definition of `pg_user_attr` catalog
- `src/include/catalog/pg_resource_attr.h`: Definition of `pg_resource_attr` catalog
- `src/include/catalog/pg_user_attr_val.h`: Definition of `pg_user_attr_val` catalog
- `src/include/catalog/pg_resource_attr_val.h`: Definition of `pg_resource_attr_val` catalog
- `src/include/catalog/pg_abac_rule.h`: Definition of `pg_abac_rule` catalog
- `src/include/catalog/pg_abac_rule_priv.h`: Definition of `pg_abac_rule_priv` catalog
- `src/include/catalog/pg_abac_env_workday.h`: Definition of `pg_abac_env_workday` catalog
- `src/include/catalog/pg_abac_env_workday.dat`: Data File for initialization of `pg_abac_env_workday` catalog
- `src/backend/catalog/catalog.c`: Routines concerned with catalog naming conventions
- `src/include/catalog/meson.build`: Build file for system catalogs
- `src/include/catalog/Makefile`: Makefile for system catalogs


### 3. ABAC enforcement

- `src/backend/executor/execMain.c`: Definition of `ExecCheckOneRelAbacPolicies` function
- `src/include/executor/executor.h`: Header file for `execMain.c`
- `src/backend/catalog/aclchk.c`: Helper functions
- `src/include/utils/acl.h`: Header file for `acl.c` and `aclchk.c`
- `src/backend/optimizer/plan/planner.c`: `ExecCheckOneRelAbacPolicies` is called for VIEWS here
