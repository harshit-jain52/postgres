ASPS: ABAC-Supported PostgreSQL
================================

This directory contains the source code distribution of ASPS — a modified version of the PostgreSQL database management system with native support for Attribute-Based Access Control (ABAC).

ASPS extends PostgreSQL’s access control framework by integrating attribute-driven policies alongside traditional Discretionary Access Control (DAC) and Role-Based Access Control (RBAC). It introduces new SQL syntax, system catalogs, and enforcement mechanisms for managing user, resource, and environmental attributes. In addition to supporting standard PostgreSQL features such as transactions, triggers, stored procedures, and user-defined functions, ASPS allows defining and enforcing ABAC rules directly within the database engine.

<!-- Copyright and license information can be found in the file COPYRIGHT. -->
