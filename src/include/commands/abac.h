/*-------------------------------------------------------------------------
 *
 * abac.h
 *	  Commands for manipulating ABAC (Attribute-Based Access Control) rules.
 *
 *
 * src/include/commands/abac.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef ABAC_H
#define ABAC_H

#include "catalog/objectaddress.h"
#include "libpq/crypt.h"
#include "nodes/parsenodes.h"
#include "parser/parse_node.h"
#include "utils/guc.h"

extern Oid CreateUserAttribute(ParseState *pstate, CreateUserAttributeStmt *stmt);
extern void DropUserAttribute(ParseState *pstate, DropUserAttributeStmt *stmt);
extern void GrantUserAttribute(ParseState *pstate, GrantUserAttributeStmt *stmt);
void AddRoleUserAttr(Oid roleid, Oid attrid, const char* value);
extern void RevokeUserAttribute(ParseState *pstate, RevokeUserAttributeStmt *stmt);
void DelRoleUserAttr(Oid roleid, Oid attrid, const char* value, const char* attr_name, const char* username);

extern Oid CreateResourceAttribute(ParseState *pstate, CreateResourceAttributeStmt *stmt);
extern void DropResourceAttribute(ParseState *pstate, DropResourceAttributeStmt *stmt);
extern void GrantResourceAttribute(ParseState *pstate, GrantResourceAttributeStmt *stmt);
void AddResourceAttr(Oid resource_id, Oid attrid, const char* value);
extern void RevokeResourceAttribute(ParseState *pstate, RevokeResourceAttributeStmt *stmt);
void DelResourceAttr(Oid resource_id, Oid attrid, const char* value, const char* attr_name);

extern void SetEnvAttribute(ParseState *pstate, SetEnvAttributeStmt *stmt);
void handle_workday(SetEnvAttributeStmt *stmt);
void handle_timewindow(SetEnvAttributeStmt *stmt);
void handle_subnet(SetEnvAttributeStmt *stmt);
void check_subnet_exists(const char *subnet_name);
void check_unique_attributes(List *attrs, const char *attr_type);

extern void CreateAbacRule(ParseState *pstate, CreateAbacRuleStmt *stmt);
void AddRuleAttr(Oid ruleid, Oid attrid, bool is_user_attr, const char* value, bool is_null);
extern void DropAbacRule(ParseState *pstate, DropAbacRuleStmt *stmt);

AclMode string_to_privilege(const char *privname);

typedef enum AbacAdminPriv
{
    ABAC_ADMIN_NONE              = 0,
    ABAC_ADMIN_CREATE_UA         = 1 << 0,
    ABAC_ADMIN_DROP_UA           = 1 << 1,
    ABAC_ADMIN_GRANT_UA          = 1 << 2,
    ABAC_ADMIN_REVOKE_UA         = 1 << 3,

    ABAC_ADMIN_CREATE_RA         = 1 << 4,
    ABAC_ADMIN_DROP_RA           = 1 << 5,
    ABAC_ADMIN_GRANT_RA          = 1 << 6,
    ABAC_ADMIN_REVOKE_RA         = 1 << 7,

    ABAC_ADMIN_CREATE_RULE       = 1 << 8,
    ABAC_ADMIN_DROP_RULE         = 1 << 9,

    ABAC_ADMIN_SET_EA            = 1 << 10
} AbacAdminPriv;
void GrantAbacAdmin(ParseState *pstate, GrantAbacAdminStmt *stmt);
void RevokeAbacAdmin(ParseState *pstate, RevokeAbacAdminStmt *stmt);
uint32 string_to_abac_admin_priv(const char *priv);
bool sgx_check_admin_priv(Oid role_oid, uint32 priv_mask);


#endif							/* ABAC_H */
