ASPS: ABAC-Supported PostgreSQL
================================

This directory contains the source code distribution of ASPS — a modified version of the PostgreSQL database management system with native support for Attribute-Based Access Control (ABAC).

ASPS extends PostgreSQL’s access control framework by integrating attribute-driven policies alongside traditional Discretionary Access Control (DAC) and Role-Based Access Control (RBAC). It introduces new SQL syntax, system catalogs, and enforcement mechanisms for managing user, resource, and environmental attributes. In addition to supporting standard PostgreSQL features such as transactions, triggers, stored procedures, and user-defined functions, ASPS allows defining and enforcing ABAC rules directly within the database engine.

Copyright and license information can be found in the file COPYRIGHT.

## ABAC-specific SQL Statements

```sql
CREATE USER_ATTRIBUTE <attr>;
CREATE RESOURCE_ATTRIBUTE <attr>;
DROP USER_ATTRIBUTE <attr> [IF EXISTS];
DROP RESOURCE_ATTRIBUTE <attr> [IF EXISTS];

GRANT USER_ATTRIBUTE { <attr> = <val> }
    TO <user>, <user>, ...;
GRANT RESOURCE_ATTRIBUTE { <attr> = <val> }
    TO <object_type> <dbobj>, <dbobj>, ...
REVOKE USER_ATTRIBUTE { <attr> = <val>|ALL }
    FROM <user>, <user>, ...;
REVOKE RESOURCE_ATTRIBUTE { <attr> = <val>|ALL }
    FROM <object_type> <dbobj>, <dbobj>, ...

SET ENV_ATTRIBUTE workday = monday, tuesday ...;
SET ENV_ATTRIBUTE worktime = <start_hr>, <start_min>, <end_hr>, <end_min>
SET ENV_ATTRIBUTE subnet =
    <name> = <cidr>,
    <name> = <cidr>, ...

CREATE ABAC_RULE name 
    FOR <operation>, <operation>, ... OF
    USER_ATTRIBUTE ( <attr>=<val>, <attr> IS [NOT] NULL, ... )
    RESOURCE_ATTRIBUTE ( <attr>=<val>, <attr> IS [NOT] NULL, ... )
    ENV_ATTRIBUTE ( <attr>=<val>, ... )
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
- `src/include/catalog/pg_abac_env_timewindow.h`: Definition of `pg_abac_env_timewindow` catalog
- `src/include/catalog/pg_abac_env_timewindow.dat`: Data File for initialization of `pg_abac_env_timewindow` catalog
- `src/include/catalog/pg_abac_env_subnet.h`: Definition of `pg_abac_env_subnet` catalog
- `src/backend/catalog/catalog.c`: Routines concerned with catalog naming conventions
- `src/include/catalog/meson.build`: Build file for system catalogs
- `src/include/catalog/Makefile`: Makefile for system catalogs
- `src/backend/catalog/heap.c`: DeleteResourceAttributeValueTuples function


### 3. ABAC enforcement

- `src/backend/executor/execMain.c`: Updated `ExecCheckOneRelPerms` function
- `src/backend/catalog/aclchk.c`: Definition and Invocations of `pg_abac_mask` function, and helper functions
- `src/include/utils/acl.h`: Header file for `acl.c` and `aclchk.c`
