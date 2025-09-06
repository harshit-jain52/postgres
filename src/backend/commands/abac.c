/*-------------------------------------------------------------------------
 *
 * ABAC.c
 *	  Commands for manipulating ABAC (Attribute-Based Access Control) rules.
 *
 * src/backend/commands/abac.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/genam.h"
#include "access/htup_details.h"
#include "access/table.h"
#include "access/xact.h"
#include "catalog/binary_upgrade.h"
#include "catalog/catalog.h"
#include "catalog/dependency.h"
#include "catalog/indexing.h"
#include "catalog/namespace.h"
#include "catalog/objectaccess.h"
#include "catalog/pg_user_attr.h"
#include "catalog/pg_user_attr_val.h"
#include "catalog/pg_resource_attr.h"
#include "catalog/pg_resource_attr_val.h"
#include "catalog/pg_abac_rule.h"
#include "catalog/pg_auth_members.h"
#include "catalog/pg_authid.h"
#include "catalog/pg_class.h"
#include "catalog/pg_database.h"
#include "catalog/pg_db_role_setting.h"
#include "commands/abac.h"
#include "commands/comment.h"
#include "commands/dbcommands.h"
#include "commands/defrem.h"
#include "commands/seclabel.h"
#include "commands/user.h"
#include "lib/qunique.h"
#include "libpq/crypt.h"
#include "miscadmin.h"
#include "storage/lmgr.h"
#include "utils/acl.h"
#include "utils/builtins.h"
#include "utils/catcache.h"
#include "utils/fmgroids.h"
#include "utils/syscache.h"
#include "utils/varlena.h"

Oid
CreateUserAttribute(ParseState *pstate, CreateUserAttributeStmt *stmt){
	Relation	pg_user_attr_rel;
	TupleDesc	pg_user_attr_dsc;
	HeapTuple	tuple;
	Datum		new_record[Natts_pg_user_attr] = {0};
	bool		new_record_nulls[Natts_pg_user_attr] = {0};
	Oid			attrib_id;

	/*
	 *TODO: check for permissions for creating attribute
	 */

	/*
	 * Check that the user is not trying to create an attribute in the reserved
	 * "pg_" namespace.
	 */
	if (IsReservedName(stmt->attribute))
		ereport(ERROR,
				(errcode(ERRCODE_RESERVED_NAME),
				 errmsg("attribute name \"%s\" is reserved",
						stmt->attribute),
				 errdetail("Attribute names starting with \"pg_\" are reserved.")));


	pg_user_attr_rel = table_open(UserAttrRelationId, RowExclusiveLock);
	pg_user_attr_dsc = RelationGetDescr(pg_user_attr_rel);

	/*
	 * Check the pg_user_attr relation to be certain the attribute doesn't already
	 * exist.
	 */

	if (OidIsValid(get_user_attr_oid(stmt->attribute, true)))
		ereport(ERROR,
			(errcode(ERRCODE_DUPLICATE_OBJECT),
				errmsg("ABAC user attribute \"%s\" already exists",
					stmt->attribute)));

	/*
	 * Build the tuple to insert. 
	 */

	attrib_id = GetNewOidWithIndex(pg_user_attr_rel, UserAttrOidIndexId,
								Anum_pg_user_attr_oid);
	new_record[Anum_pg_user_attr_oid - 1] = ObjectIdGetDatum(attrib_id);

	new_record[Anum_pg_user_attr_attrib_name- 1] = DirectFunctionCall1(namein, CStringGetDatum(stmt->attribute));

	tuple = heap_form_tuple(pg_user_attr_dsc, new_record, new_record_nulls);

	/*
	 * Insert new record in the pg_user_attr table
	 */
	CatalogTupleInsert(pg_user_attr_rel, tuple);

	/*
	 * Advance command counter so we can see new record
	 */
	CommandCounterIncrement();

	/*
	 * Close pg_user_attr, but keep lock till commit.
	 */
	table_close(pg_user_attr_rel, NoLock);

	return attrib_id;
}

Oid
CreateResourceAttribute(ParseState *pstate, CreateResourceAttributeStmt *stmt){
	Relation	pg_resource_attr_rel;
	TupleDesc	pg_resource_attr_dsc;
	HeapTuple	tuple;
	Datum		new_record[Natts_pg_resource_attr] = {0};
	bool		new_record_nulls[Natts_pg_resource_attr] = {0};
	Oid			attrib_id;

	/*
	 *TODO: check for permissions for creating attribute
	 */

	/*
	 * Check that the user is not trying to create an attribute in the reserved
	 * "pg_" namespace.
	 */
	if (IsReservedName(stmt->attribute))
		ereport(ERROR,
				(errcode(ERRCODE_RESERVED_NAME),
				 errmsg("attribute name \"%s\" is reserved",
						stmt->attribute),
				 errdetail("Attribute names starting with \"pg_\" are reserved.")));


	pg_resource_attr_rel = table_open(ResourceAttrRelationId, RowExclusiveLock);
	pg_resource_attr_dsc = RelationGetDescr(pg_resource_attr_rel);

	/*
	 * Check the pg_resource_attr relation to be certain the attribute doesn't already
	 * exist.
	 */

	if (OidIsValid(get_resource_attr_oid(stmt->attribute, true)))
		ereport(ERROR,
			(errcode(ERRCODE_DUPLICATE_OBJECT),
				errmsg("ABAC resource attribute \"%s\" already exists",
					stmt->attribute)));

	/*
	 * Build the tuple to insert. 
	 */

	attrib_id = GetNewOidWithIndex(pg_resource_attr_rel, ResourceAttrOidIndexId,
								Anum_pg_resource_attr_oid);
	new_record[Anum_pg_resource_attr_oid - 1] = ObjectIdGetDatum(attrib_id);

	new_record[Anum_pg_resource_attr_attrib_name- 1] = DirectFunctionCall1(namein, CStringGetDatum(stmt->attribute));

	tuple = heap_form_tuple(pg_resource_attr_dsc, new_record, new_record_nulls);

	/*
	 * Insert new record in the pg_resource_attr table
	 */
	CatalogTupleInsert(pg_resource_attr_rel, tuple);

	/*
	 * Advance command counter so we can see new record
	 */
	CommandCounterIncrement();

	/*
	 * Close pg_resource_attr, but keep lock till commit.
	 */
	table_close(pg_resource_attr_rel, NoLock);

	return attrib_id;
}


void DropUserAttribute(ParseState *pstate, DropUserAttributeStmt *stmt){
	/* This function is not implemented in this version */
	ereport(ERROR,
			(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
			 errmsg("DROP USER ATTRIBUTE is not supported in this version. Attribute: \"%s\"", stmt->attribute)));
}

void DropResourceAttribute(ParseState *pstate, DropResourceAttributeStmt *stmt){
	/* This function is not implemented in this version */
	ereport(ERROR,
			(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
			 errmsg("DROP RESOURCE ATTRIBUTE is not supported in this version. Attribute: \"%s\"", stmt->attribute)));	 
}

void AddRoleUserAttr(Oid roleid, Oid attrid, const char* value)
{
	Relation	pg_user_attr_val_rel;
	TupleDesc	pg_user_attr_val_dsc;
	HeapTuple	oldtuple;
	HeapTuple	newtuple;
	ScanKeyData	skey[2];
	SysScanDesc	scan;
	Datum		roleid_datum;
	Datum		attrid_datum;
	Datum		value_datum;
	Datum		new_record[Natts_pg_user_attr_val] = {0};
	bool		new_record_nulls[Natts_pg_user_attr_val] = {0};
	bool		new_record_repl[Natts_pg_user_attr_val] = {0};

	pg_user_attr_val_rel = table_open(UserAttrValRelationId, RowExclusiveLock);
	pg_user_attr_val_dsc = RelationGetDescr(pg_user_attr_val_rel);

	roleid_datum = ObjectIdGetDatum(roleid);
	attrid_datum = ObjectIdGetDatum(attrid);
	value_datum = DirectFunctionCall1(textin, CStringGetDatum(value));

	ScanKeyInit(&skey[0],
				Anum_pg_user_attr_val_user_id,
				BTEqualStrategyNumber, F_OIDEQ,
				roleid_datum);
	ScanKeyInit(&skey[1],
				Anum_pg_user_attr_val_attr_id,
				BTEqualStrategyNumber, F_OIDEQ,
				attrid_datum);
	scan = systable_beginscan(pg_user_attr_val_rel, UserAttrValPkeyIndexId, true,  
							  NULL, 2, skey);  
	oldtuple = systable_getnext(scan); 

	if(HeapTupleIsValid(oldtuple)){
		new_record[Anum_pg_user_attr_val_value - 1] = value_datum;
		new_record_repl[Anum_pg_user_attr_val_value - 1] = true;
		newtuple = heap_modify_tuple(oldtuple, pg_user_attr_val_dsc,
									  new_record,
									  new_record_nulls, new_record_repl);
		CatalogTupleUpdate(pg_user_attr_val_rel, &newtuple->t_self, newtuple);  
	}
	else{
		new_record[Anum_pg_user_attr_val_attr_id - 1] = attrid_datum;
		new_record[Anum_pg_user_attr_val_user_id - 1] = roleid_datum;
		new_record[Anum_pg_user_attr_val_value - 1] = value_datum;

		newtuple = heap_form_tuple(pg_user_attr_val_dsc, new_record, new_record_nulls);
		CatalogTupleInsert(pg_user_attr_val_rel, newtuple);
	}

	heap_freetuple(newtuple);
	systable_endscan(scan);
	table_close(pg_user_attr_val_rel, NoLock);
}

void AddRelResourceAttr(Oid relid, Oid attrid, const char* value)
{
	Relation	pg_resource_attr_val_rel;
	TupleDesc	pg_resource_attr_val_dsc;
	HeapTuple	oldtuple;
	HeapTuple	newtuple;
	ScanKeyData	skey[2];  
	SysScanDesc	scan;
	Datum		relid_datum;
	Datum		attrid_datum;
	Datum		value_datum;
	Datum		new_record[Natts_pg_resource_attr_val] = {0};  
	bool		new_record_nulls[Natts_pg_resource_attr_val] = {0};  
	bool		new_record_repl[Natts_pg_resource_attr_val] = {0};  
  
	pg_resource_attr_val_rel = table_open(ResourceAttrValRelationId, RowExclusiveLock);  
	pg_resource_attr_val_dsc = RelationGetDescr(pg_resource_attr_val_rel);  

	relid_datum = ObjectIdGetDatum(relid);
	attrid_datum = ObjectIdGetDatum(attrid);
	value_datum = DirectFunctionCall1(textin, CStringGetDatum(value));

	ScanKeyInit(&skey[0],
				Anum_pg_resource_attr_val_resource_id,
				BTEqualStrategyNumber, F_OIDEQ,
				relid_datum);
	ScanKeyInit(&skey[1],
				Anum_pg_resource_attr_val_attr_id,
				BTEqualStrategyNumber, F_OIDEQ,
				attrid_datum);

	scan = systable_beginscan(pg_resource_attr_val_rel, ResourceAttrValPkeyIndexId, true,
							  NULL, 2, skey);  
	oldtuple = systable_getnext(scan);  
  
	if (HeapTupleIsValid(oldtuple))
	{
		new_record[Anum_pg_resource_attr_val_value - 1] = value_datum;
		new_record_repl[Anum_pg_resource_attr_val_value - 1] = true;

		newtuple = heap_modify_tuple(oldtuple, pg_resource_attr_val_dsc,
									 new_record, new_record_nulls, new_record_repl);
		CatalogTupleUpdate(pg_resource_attr_val_rel, &newtuple->t_self, newtuple);
	}  
	else
	{
		new_record[Anum_pg_resource_attr_val_resource_id - 1] = relid_datum;
		new_record[Anum_pg_resource_attr_val_attr_id - 1] = attrid_datum;
		new_record[Anum_pg_resource_attr_val_value - 1] = value_datum;

		newtuple = heap_form_tuple(pg_resource_attr_val_dsc, new_record, new_record_nulls);
		CatalogTupleInsert(pg_resource_attr_val_rel, newtuple);
	}

	heap_freetuple(newtuple);  
	systable_endscan(scan);  
	table_close(pg_resource_attr_val_rel, NoLock);  
}

void GrantUserAttribute(ParseState *pstate, GrantUserAttributeStmt *stmt){
	Relation	pg_authid_rel;
	Relation	pg_user_attr_rel;
	List	   *grantee_ids;
	ListCell   *item;

	grantee_ids = roleSpecsToIds(stmt->grantees);

	/*
	 *TODO: check for permissions for grantor
	 */

	pg_authid_rel = table_open(AuthIdRelationId, AccessShareLock);
	pg_user_attr_rel = table_open(UserAttrRelationId, AccessShareLock);

	foreach(item, stmt->grantees)
	{
		AccessPriv *priv = (AccessPriv *) lfirst(item);
		char	   *rolename = priv->priv_name;
		Oid			roleid;
		Oid			attrid;

		roleid = get_role_oid(rolename, false);
		attrid = get_user_attr_oid(stmt->attribute, false);
		AddRoleUserAttr(roleid, attrid, stmt->value);
	}

	/*
	 * Close pg_authid_rel and pg_user_attr_rel, but keep lock till commit.
	 */
	table_close(pg_authid_rel, NoLock);
	table_close(pg_user_attr_rel, NoLock);
}

void GrantResourceAttribute(ParseState *pstate, GrantResourceAttributeStmt *stmt){
	Relation	pg_class_rel;  
	Relation	pg_resource_attr_rel;  
	ListCell   *item;  
  
	pg_class_rel = table_open(RelationRelationId, AccessShareLock);  
	pg_resource_attr_rel = table_open(ResourceAttrRelationId, AccessShareLock);  
  
	foreach(item, stmt->grantees)  
	{  
		RangeVar   *relvar = (RangeVar *) lfirst(item);  
		char	   *relname = relvar->relname;  
		Oid			relid;  
		Oid			attrid;  
		
		/*
		 *TODO: check for permissions for grantor
		 */

		relid = RangeVarGetRelid(relvar, NoLock, false);
		attrid = get_resource_attr_oid(stmt->attribute, false);
		AddRelResourceAttr(relid, attrid, stmt->value);
	}

	/*
	 * Close relations, but keep lock till commit.  
	 */  
	table_close(pg_class_rel, NoLock);  
	table_close(pg_resource_attr_rel, NoLock); 
}

void RevokeUserAttribute(ParseState *pstate, RevokeUserAttributeStmt *stmt){
	/* This function is not implemented in this version */
	ereport(ERROR,
			(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
			 errmsg("REVOKE USER ATTRIBUTE is not supported in this version. Attribute: \"%s\", Value: \"%s\"", stmt->attribute, stmt->value)));
}

void RevokeResourceAttribute(ParseState *pstate, RevokeResourceAttributeStmt *stmt){
	/* This function is not implemented in this version */
	ereport(ERROR,
			(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
			 errmsg("REVOKE RESOURCE ATTRIBUTE is not supported in this version. Attribute: \"%s\", Value: \"%s\"", stmt->attribute, stmt->value)));
}

// TODO: handle privileges
void
CreateAbacRule(ParseState *pstate, CreateAbacRuleStmt *stmt)  
{  
	Relation	pg_abac_rule_rel;    
	TupleDesc	pg_abac_rule_dsc;    
	HeapTuple	tuple;  
	ScanKeyData	skey[1];  
	SysScanDesc	scan;  
	Datum		values[Natts_pg_abac_rule];    
	bool		nulls[Natts_pg_abac_rule];    
	Datum		rulename_datum;  
	Datum		is_user_datum;  
	List	   *user_attrs;    
	List	   *resource_attrs;    
	ListCell   *lc;    
    
	/* Extract user and resource attribute clauses */    
	user_attrs = (List *) linitial(stmt->attribute_clause);    
	resource_attrs = (List *) lsecond(stmt->attribute_clause);    
	rulename_datum = DirectFunctionCall1(namein, CStringGetDatum(stmt->rule_name));
    
	pg_abac_rule_rel = table_open(AbacRuleRelationId, RowExclusiveLock);    
	pg_abac_rule_dsc = RelationGetDescr(pg_abac_rule_rel);    
  
	/* Check if rule with same name already exists */  
	ScanKeyInit(&skey[0],  
				Anum_pg_abac_rule_rulename,  
				BTEqualStrategyNumber, F_NAMEEQ,  
				rulename_datum);  
  
	scan = systable_beginscan(pg_abac_rule_rel, AbacRulePkeyIndexId, true,  
							  NULL, 1, skey);  
	  
	if (HeapTupleIsValid(systable_getnext(scan)))  {  
		systable_endscan(scan);  
		table_close(pg_abac_rule_rel, NoLock);  
		ereport(ERROR,  
				(errcode(ERRCODE_DUPLICATE_OBJECT),  
				 errmsg("ABAC rule \"%s\" already exists", stmt->rule_name)));  
	}
	  
	systable_endscan(scan);


	/* Process user attributes */  
	if (user_attrs != NIL)  {  
		is_user_datum = BoolGetDatum(true);
		foreach(lc, user_attrs)  {  
			DefElem    *def = (DefElem *) lfirst(lc);  
			char	   *attr_name = def->defname;  
			char	   *attr_value = strVal(def->arg);  
			Oid			attr_id;  
  
			attr_id = get_user_attr_oid(attr_name, false);  
  
			memset(values, 0, sizeof(values));  
			memset(nulls, false, sizeof(nulls));  
  
			values[Anum_pg_abac_rule_rulename - 1] = rulename_datum;
			values[Anum_pg_abac_rule_attr_id - 1] = ObjectIdGetDatum(attr_id);  
			values[Anum_pg_abac_rule_is_user_attr - 1] = is_user_datum;  
			values[Anum_pg_abac_rule_value - 1] = CStringGetTextDatum(attr_value);  
  
			tuple = heap_form_tuple(pg_abac_rule_dsc, values, nulls);  
			CatalogTupleInsert(pg_abac_rule_rel, tuple);  
			heap_freetuple(tuple);  
		}  
	}  
  
	/* Process resource attributes */  
	if (resource_attrs != NIL)  {
		is_user_datum = BoolGetDatum(false);
		foreach(lc, resource_attrs)  {
			DefElem    *def = (DefElem *) lfirst(lc);
			char	   *attr_name = def->defname;
			char	   *attr_value = strVal(def->arg);
			Oid			attr_id;  
  
			attr_id = get_resource_attr_oid(attr_name, false);  
  
			memset(values, 0, sizeof(values));  
			memset(nulls, false, sizeof(nulls));  
  
			values[Anum_pg_abac_rule_rulename - 1] = rulename_datum;
			values[Anum_pg_abac_rule_attr_id - 1] = ObjectIdGetDatum(attr_id);
			values[Anum_pg_abac_rule_is_user_attr - 1] = is_user_datum;
			values[Anum_pg_abac_rule_value - 1] = CStringGetTextDatum(attr_value);

			tuple = heap_form_tuple(pg_abac_rule_dsc, values, nulls);
			CatalogTupleInsert(pg_abac_rule_rel, tuple);
			heap_freetuple(tuple);
		}  
	}  
  
	/* Close the catalog */  
	table_close(pg_abac_rule_rel, NoLock);    
}

void DropAbacRule(ParseState *pstate, DropAbacRuleStmt *stmt){
	/* This function is not implemented in this version */
	ereport(ERROR,
			(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
			 errmsg("DROP ABAC RULE is not supported in this version. Rule: \"%s\"", stmt->rule_name)));
}