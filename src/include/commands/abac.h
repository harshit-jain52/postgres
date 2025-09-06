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

extern Oid CreateResourceAttribute(ParseState *pstate, CreateResourceAttributeStmt *stmt);
extern void DropResourceAttribute(ParseState *pstate, DropResourceAttributeStmt *stmt);
extern void GrantResourceAttribute(ParseState *pstate, GrantResourceAttributeStmt *stmt);
void AddRelResourceAttr(Oid relid, Oid attrid, const char* value);
extern void RevokeResourceAttribute(ParseState *pstate, RevokeResourceAttributeStmt *stmt);

extern void CreateAbacRule(ParseState *pstate, CreateAbacRuleStmt *stmt);
extern void DropAbacRule(ParseState *pstate, DropAbacRuleStmt *stmt);

#endif							/* ABAC_H */
