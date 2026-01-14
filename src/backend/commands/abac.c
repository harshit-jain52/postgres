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
#include "catalog/pg_abac_env_subnet.h"
#include "catalog/pg_abac_env_timewindow.h"
#include "catalog/pg_abac_env_workday.h"
#include "catalog/pg_abac_rule.h"
#include "catalog/pg_abac_rule_priv.h"
#include "catalog/pg_auth_members.h"
#include "catalog/pg_authid.h"
#include "catalog/pg_class.h"
#include "catalog/pg_database.h"
#include "catalog/pg_db_role_setting.h"
#include "catalog/pg_proc.h"
#include "commands/abac.h"
#include "commands/comment.h"
#include "commands/dbcommands.h"
#include "commands/defrem.h"
#include "commands/seclabel.h"
#include "commands/user.h"
#include "nodes/nodes.h"
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
	Oid			currentUserId = GetUserId();

	/*
	 * Creator must be a superuser
	 */
	if (!superuser_arg(currentUserId))
		ereport(ERROR,
				(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
				 errmsg("must be superuser to create user attribute")));

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
	Oid			currentUserId = GetUserId();

	/*
	 * Creator must be a superuser
	 */
	if (!superuser_arg(currentUserId))
		ereport(ERROR,
				(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
				 errmsg("must be superuser to create resource attribute")));

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


void  
DropUserAttribute(ParseState *pstate, DropUserAttributeStmt *stmt)  
{  
    Relation    pg_user_attr_rel;  
    Relation    pg_user_attr_val_rel;  
    Relation    pg_abac_rule_rel;  
    ScanKeyData skey[1];  
    SysScanDesc scan;  
    HeapTuple   tuple;  
    Oid         attr_oid;  
    Oid         currentUserId = GetUserId();  
    bool        attr_in_use = false;  
  
    /* Only superusers can drop user attributes */  
    if (!superuser_arg(currentUserId))  
        ereport(ERROR,  
                (errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),  
                 errmsg("must be superuser to drop user attribute")));  
  
    attr_oid = get_user_attr_oid(stmt->attribute, stmt->missing_ok);
	
	if (!OidIsValid(attr_oid) && stmt->missing_ok)
		return;
  
    /* Check if attribute is used in any ABAC rules */  
    pg_abac_rule_rel = table_open(AbacRuleRelationId, AccessShareLock);  
  
    ScanKeyInit(&skey[0],  
                Anum_pg_abac_rule_attr_id,  
                BTEqualStrategyNumber, F_OIDEQ,  
                ObjectIdGetDatum(attr_oid));  
  
    scan = systable_beginscan(pg_abac_rule_rel, AbacRulePkeyIndexId, true,  
                              NULL, 1, skey);  
  
    while (HeapTupleIsValid(tuple = systable_getnext(scan)))  
    {  
        Form_pg_abac_rule rule = (Form_pg_abac_rule) GETSTRUCT(tuple);  
          
        if (rule->is_user_attr)  
        {  
            attr_in_use = true;  
            break;  
        }  
    }  
  
    systable_endscan(scan);  
    table_close(pg_abac_rule_rel, AccessShareLock);  
  
    /* If attribute is in use, throw an error */  
    if (attr_in_use)  
        ereport(ERROR,  
                (errcode(ERRCODE_DEPENDENT_OBJECTS_STILL_EXIST),  
                 errmsg("cannot drop user attribute \"%s\" because it is used in ABAC rules",  
                        stmt->attribute),  
                 errhint("Drop the ABAC rules using this attribute first.")));  
  
    /* Delete all attribute value assignments from pg_user_attr_val */  
    pg_user_attr_val_rel = table_open(UserAttrValRelationId, RowExclusiveLock);  
  
    ScanKeyInit(&skey[0],  
                Anum_pg_user_attr_val_attr_id,  
                BTEqualStrategyNumber, F_OIDEQ,  
                ObjectIdGetDatum(attr_oid));  
  
    scan = systable_beginscan(pg_user_attr_val_rel, UserAttrValPkeyIndexId, true,  
                              NULL, 1, skey);  
  
    while (HeapTupleIsValid(tuple = systable_getnext(scan)))  
        CatalogTupleDelete(pg_user_attr_val_rel, &tuple->t_self);  
  
    systable_endscan(scan);  
    table_close(pg_user_attr_val_rel, RowExclusiveLock);  
  
    /* Delete the attribute definition from pg_user_attr */  
    pg_user_attr_rel = table_open(UserAttrRelationId, RowExclusiveLock);  
  
    tuple = SearchSysCache1(USERATTRID, ObjectIdGetDatum(attr_oid));  
    if (!HeapTupleIsValid(tuple))  
        elog(ERROR, "cache lookup failed for user attribute %u", attr_oid);  
  
    CatalogTupleDelete(pg_user_attr_rel, &tuple->t_self);  
  
    ReleaseSysCache(tuple);  
    table_close(pg_user_attr_rel, RowExclusiveLock);  
  
    /* Make changes visible */  
    CommandCounterIncrement();  
}

void DropResourceAttribute(ParseState *pstate, DropResourceAttributeStmt *stmt){
	Relation    pg_resource_attr_rel;  
    Relation    pg_resource_attr_val_rel;  
    Relation    pg_abac_rule_rel;  
    ScanKeyData skey[1];  
    SysScanDesc scan;  
    HeapTuple   tuple;  
    Oid         attr_oid;  
    Oid         currentUserId = GetUserId();  
    bool        attr_in_use = false;  
  
    /* Only superusers can drop resource attributes */  
    if (!superuser_arg(currentUserId))  
        ereport(ERROR,  
                (errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),  
                 errmsg("must be superuser to drop resource attribute")));  
  
    attr_oid = get_resource_attr_oid(stmt->attribute, stmt->missing_ok);
	
	if (!OidIsValid(attr_oid) && stmt->missing_ok)
		return;

    /* Check if attribute is used in any ABAC rules */  
    pg_abac_rule_rel = table_open(AbacRuleRelationId, AccessShareLock);  
  
    ScanKeyInit(&skey[0],  
                Anum_pg_abac_rule_attr_id,  
                BTEqualStrategyNumber, F_OIDEQ,  
                ObjectIdGetDatum(attr_oid));  
  
    scan = systable_beginscan(pg_abac_rule_rel, AbacRulePkeyIndexId, true,  
                              NULL, 1, skey);  
  
    while (HeapTupleIsValid(tuple = systable_getnext(scan)))  
    {  
        Form_pg_abac_rule rule = (Form_pg_abac_rule) GETSTRUCT(tuple);  
          
        if (!rule->is_user_attr)  
        {  
            attr_in_use = true;  
            break;  
        }  
    }  
  
    systable_endscan(scan);  
    table_close(pg_abac_rule_rel, AccessShareLock);  
  
    /* If attribute is in use, throw an error */  
    if (attr_in_use)  
        ereport(ERROR,  
                (errcode(ERRCODE_DEPENDENT_OBJECTS_STILL_EXIST),  
                 errmsg("cannot drop resource attribute \"%s\" because it is used in ABAC rules",  
                        stmt->attribute),  
                 errhint("Drop the ABAC rules using this attribute first.")));  

    /* Delete all attribute value assignments from pg_resource_attr_val */  
    pg_resource_attr_val_rel = table_open(ResourceAttrValRelationId, RowExclusiveLock);  

    ScanKeyInit(&skey[0],
                Anum_pg_resource_attr_val_attr_id,
                BTEqualStrategyNumber, F_OIDEQ,
                ObjectIdGetDatum(attr_oid));

    scan = systable_beginscan(pg_resource_attr_val_rel, ResourceAttrValPkeyIndexId, true,
                              NULL, 1, skey);

    while (HeapTupleIsValid(tuple = systable_getnext(scan)))
        CatalogTupleDelete(pg_resource_attr_val_rel, &tuple->t_self);

    systable_endscan(scan);
    table_close(pg_resource_attr_val_rel, RowExclusiveLock);

    /* Delete the attribute definition from pg_resource_attr */
    pg_resource_attr_rel = table_open(ResourceAttrRelationId, RowExclusiveLock);

    tuple = SearchSysCache1(RESOURCEATTRID, ObjectIdGetDatum(attr_oid));
    if (!HeapTupleIsValid(tuple))
        elog(ERROR, "cache lookup failed for resource attribute %u", attr_oid);

    CatalogTupleDelete(pg_resource_attr_rel, &tuple->t_self);

    ReleaseSysCache(tuple);
    table_close(pg_resource_attr_rel, RowExclusiveLock);

    /* Make changes visible */  
    CommandCounterIncrement();  	 
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

void AddResourceAttr(Oid resource_id, Oid attrid, const char* value)
{
	Relation	pg_resource_attr_val_rel;
	TupleDesc	pg_resource_attr_val_dsc;
	HeapTuple	oldtuple;
	HeapTuple	newtuple;
	ScanKeyData	skey[2];  
	SysScanDesc	scan;
	Datum		resource_id_datum;
	Datum		attrid_datum;
	Datum		value_datum;
	Datum		new_record[Natts_pg_resource_attr_val] = {0};  
	bool		new_record_nulls[Natts_pg_resource_attr_val] = {0};  
	bool		new_record_repl[Natts_pg_resource_attr_val] = {0};  
  
	pg_resource_attr_val_rel = table_open(ResourceAttrValRelationId, RowExclusiveLock);  
	pg_resource_attr_val_dsc = RelationGetDescr(pg_resource_attr_val_rel);  

	resource_id_datum = ObjectIdGetDatum(resource_id);
	attrid_datum = ObjectIdGetDatum(attrid);
	value_datum = DirectFunctionCall1(textin, CStringGetDatum(value));

	ScanKeyInit(&skey[0],
				Anum_pg_resource_attr_val_resource_id,
				BTEqualStrategyNumber, F_OIDEQ,
				resource_id_datum);
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
		new_record[Anum_pg_resource_attr_val_resource_id - 1] = resource_id_datum;
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
	ListCell   *item;
	Oid			currentUserId = GetUserId();
	HeapTuple   tuple;  
	Form_pg_authid authform;

	pg_authid_rel = table_open(AuthIdRelationId, AccessShareLock);
	pg_user_attr_rel = table_open(UserAttrRelationId, AccessShareLock);

	foreach(item, stmt->grantees)
	{
		AccessPriv *priv = (AccessPriv *) lfirst(item);
		char	   *rolename = priv->priv_name;
		Oid			roleid;
		Oid			attrid;

		roleid = get_role_oid(rolename, false);

		/* Role should be a user, that is, have LOGIN privilege */
		tuple = SearchSysCache1(AUTHOID, ObjectIdGetDatum(roleid));
		if (!HeapTupleIsValid(tuple))
			ereport(ERROR,
					(errcode(ERRCODE_UNDEFINED_OBJECT),
					errmsg("role with OID %u does not exist", roleid)));
		
		authform = (Form_pg_authid) GETSTRUCT(tuple);  
		
		if (!authform->rolcanlogin)  
		{  
			ReleaseSysCache(tuple);
			ereport(ERROR,  
					(errcode(ERRCODE_WRONG_OBJECT_TYPE),  
					errmsg("cannot grant attribute to role \"%s\"", rolename),  
					errdetail("Only roles with LOGIN privilege can receive user attributes.")));  
		}
		ReleaseSysCache(tuple);

		/*
		* Grantor must be a superuser or the role admin
		*/
		if (!superuser_arg(currentUserId) && !is_admin_of_role(currentUserId, roleid))
			ereport(ERROR,
					(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
					 errmsg("must be superuser or role admin to grant attribute to role \"%s\"",
							rolename)));
		
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
    Relation    pg_class_rel;
    Relation    pg_resource_attr_rel;
    ListCell   *item;
    Oid         currentUserId = GetUserId();

    pg_class_rel = table_open(RelationRelationId, AccessShareLock);
    pg_resource_attr_rel = table_open(ResourceAttrRelationId, AccessShareLock);

    foreach(item, stmt->grantees)
    {
        Oid         resource_id;
        Oid         attrid;

        switch (stmt->resource_type)
        {
            case OBJECT_TABLE:
            case OBJECT_VIEW:
            case OBJECT_SEQUENCE:
            {
                RangeVar   *relvar = (RangeVar *) lfirst(item);
                resource_id = RangeVarGetRelid(relvar, NoLock, false);
                  
                /*  
                 * Grantor must be a superuser or the resource owner  
                 */
                if(!superuser_arg(currentUserId) &&
                   !object_ownercheck(RelationRelationId, resource_id, currentUserId))
                    ereport(ERROR,
                            (errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
                             errmsg("must be superuser or resource owner to grant attribute to relation \"%s\"",
                                 relvar->relname)));
                break;
            }  
            case OBJECT_FUNCTION:
			{
				ObjectWithArgs *fwa = (ObjectWithArgs *) lfirst(item);
				ObjectAddress address;

				address = get_object_address(OBJECT_FUNCTION,
											(Node *) fwa,
											NULL,
											AccessShareLock,
											false);

				resource_id = address.objectId;

				if (!superuser_arg(currentUserId) &&
					!object_ownercheck(ProcedureRelationId,
									resource_id,
									currentUserId))
					ereport(ERROR,
							(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
							errmsg("must be superuser or function owner to grant attribute to function")));
				break;
			}
            default:
                elog(ERROR, "unsupported resource type: %d", stmt->resource_type);
        }  

        attrid = get_resource_attr_oid(stmt->attribute, false);
        AddResourceAttr(resource_id, attrid, stmt->value);
    }

    /*
     * Close relations, but keep lock till commit.
     */
    table_close(pg_class_rel, NoLock);
    table_close(pg_resource_attr_rel, NoLock);
}

void
DelRoleUserAttr(Oid roleid, Oid attrid, const char* value, const char* attr_name, const char* username)
{  
    Relation    pg_user_attr_val_rel;
	TupleDesc   pg_user_attr_val_dsc;
    ScanKeyData skey[2];
    SysScanDesc scan;
    HeapTuple   tuple;
    bool        found = false;
  
    pg_user_attr_val_rel = table_open(UserAttrValRelationId, RowExclusiveLock);
    pg_user_attr_val_dsc = RelationGetDescr(pg_user_attr_val_rel);

    ScanKeyInit(&skey[0],
                Anum_pg_user_attr_val_user_id,
                BTEqualStrategyNumber, F_OIDEQ,
                ObjectIdGetDatum(roleid));
    ScanKeyInit(&skey[1],
                Anum_pg_user_attr_val_attr_id,
                BTEqualStrategyNumber, F_OIDEQ,
                ObjectIdGetDatum(attrid));

    scan = systable_beginscan(pg_user_attr_val_rel, UserAttrValPkeyIndexId,
                              true, NULL, 2, skey);
    tuple = systable_getnext(scan);

    if (HeapTupleIsValid(tuple))
    {
        Datum       value_datum;
        text       *value_text;
        char       *actual_value;
        bool        isnull;
        value_datum = heap_getattr(tuple, Anum_pg_user_attr_val_value,
                                   pg_user_attr_val_dsc, &isnull);
        if (!isnull)
        {
            value_text = DatumGetTextP(value_datum);
            actual_value = text_to_cstring(value_text);
            if (strcmp(value, "all") == 0 || strcmp(actual_value, value) == 0)
            {
                CatalogTupleDelete(pg_user_attr_val_rel, &tuple->t_self);
                found = true;
            }
            pfree(actual_value);
        }
    }

    systable_endscan(scan);
    table_close(pg_user_attr_val_rel, NoLock);
	
    /* Issue a warning if no matching entry was found */
    if (!found)
    {
        ereport(WARNING,
                (errmsg("attribute \"%s\" with specified value not found for role %s",
                        attr_name, username)));
    }
}

void
DelResourceAttr(Oid resource_id, Oid attrid, const char* value, const char* attr_name)
{
	Relation    pg_resource_attr_val_rel;
	TupleDesc   pg_resource_attr_val_dsc;
    ScanKeyData skey[2];
    SysScanDesc scan;
    HeapTuple   tuple;
    bool        found = false;
  
    pg_resource_attr_val_rel = table_open(ResourceAttrValRelationId, RowExclusiveLock);
    pg_resource_attr_val_dsc = RelationGetDescr(pg_resource_attr_val_rel);

    ScanKeyInit(&skey[0],
                Anum_pg_resource_attr_val_resource_id,
                BTEqualStrategyNumber, F_OIDEQ,
                ObjectIdGetDatum(resource_id));
    ScanKeyInit(&skey[1],
                Anum_pg_resource_attr_val_attr_id,
                BTEqualStrategyNumber, F_OIDEQ,
                ObjectIdGetDatum(attrid));

    scan = systable_beginscan(pg_resource_attr_val_rel, ResourceAttrValPkeyIndexId,
                              true, NULL, 2, skey);
    tuple = systable_getnext(scan);

    if (HeapTupleIsValid(tuple))
    {
        Datum       value_datum;
        text       *value_text;
        char       *actual_value;
        bool        isnull;
        value_datum = heap_getattr(tuple, Anum_pg_resource_attr_val_value,
                                   pg_resource_attr_val_dsc, &isnull);
        if (!isnull)
        {
            value_text = DatumGetTextP(value_datum);
            actual_value = text_to_cstring(value_text);
            if (strcmp(value, "all") == 0 || strcmp(actual_value, value) == 0)
            {
                CatalogTupleDelete(pg_resource_attr_val_rel, &tuple->t_self);
                found = true;
            }
            pfree(actual_value);
        }
    }

    systable_endscan(scan);
    table_close(pg_resource_attr_val_rel, NoLock);
	
    /* Issue a warning if no matching entry was found */
    if (!found)
    {
        ereport(WARNING,
                (errmsg("attribute \"%s\" with specified value not found for the resource", attr_name)));
    }
}

void RevokeUserAttribute(ParseState *pstate, RevokeUserAttributeStmt *stmt){
	Relation	pg_authid_rel;
	Relation	pg_user_attr_rel;
	ListCell   *item;
	Oid			currentUserId = GetUserId();

	pg_authid_rel = table_open(AuthIdRelationId, AccessShareLock);
	pg_user_attr_rel = table_open(UserAttrRelationId, AccessShareLock);

	foreach(item, stmt->grantees)
	{
		AccessPriv *priv = (AccessPriv *) lfirst(item);
		char	   *rolename = priv->priv_name;
		Oid			roleid;
		Oid			attrid;

		roleid = get_role_oid(rolename, false);
		/*
		* Revoker must be a superuser or the role admin
		*/
		if (!superuser_arg(currentUserId) && !is_admin_of_role(currentUserId, roleid))
			ereport(ERROR,
					(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
					 errmsg("must be superuser or role admin to revoke attribute from role \"%s\"",
							rolename)));
		
		attrid = get_user_attr_oid(stmt->attribute, false);
		DelRoleUserAttr(roleid, attrid, stmt->value, stmt->attribute, rolename);
	}

	/*
	 * Close pg_authid_rel and pg_user_attr_rel, but keep lock till commit.
	 */
	table_close(pg_authid_rel, NoLock);
	table_close(pg_user_attr_rel, NoLock);	
}

void RevokeResourceAttribute(ParseState *pstate, RevokeResourceAttributeStmt *stmt){
	Relation    pg_class_rel;
    Relation    pg_resource_attr_rel;
    ListCell   *item;
    Oid         currentUserId = GetUserId();

    pg_class_rel = table_open(RelationRelationId, AccessShareLock);
    pg_resource_attr_rel = table_open(ResourceAttrRelationId, AccessShareLock);

    foreach(item, stmt->grantees)
    {
        Oid         resource_id;
        Oid         attrid;

        switch (stmt->resource_type)
        {
            case OBJECT_TABLE:
            case OBJECT_VIEW:
            case OBJECT_SEQUENCE:
            {
                RangeVar   *relvar = (RangeVar *) lfirst(item);
                resource_id = RangeVarGetRelid(relvar, NoLock, false);
                  
                /*  
                 * Revoker must be a superuser or the resource owner  
                 */
                if(!superuser_arg(currentUserId) &&
                   !object_ownercheck(RelationRelationId, resource_id, currentUserId))
                    ereport(ERROR,
                            (errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
                             errmsg("must be superuser or resource owner to revoke attribute from relation \"%s\"",
                                 relvar->relname)));
                break;
            }  
            case OBJECT_FUNCTION:
			{
				ObjectWithArgs *fwa = (ObjectWithArgs *) lfirst(item);
				ObjectAddress address;

				address = get_object_address(OBJECT_FUNCTION,
											(Node *) fwa,
											NULL,
											AccessShareLock,
											false);

				resource_id = address.objectId;

				if (!superuser_arg(currentUserId) &&
					!object_ownercheck(ProcedureRelationId,
									resource_id,
									currentUserId))
					ereport(ERROR,
							(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
							errmsg("must be superuser or function owner to revoke attribute from function")));
				break;
			}
            default:
                elog(ERROR, "unsupported resource type: %d", stmt->resource_type);
        }  

        attrid = get_resource_attr_oid(stmt->attribute, false);
        DelResourceAttr(resource_id, attrid, stmt->value, stmt->attribute);
    }

    /*
     * Close relations, but keep lock till commit.
     */
    table_close(pg_class_rel, NoLock);
    table_close(pg_resource_attr_rel, NoLock);
}

void SetEnvAttribute(ParseState *pstate, SetEnvAttributeStmt *stmt){
	Oid			currentUserId = GetUserId();

	if(!superuser_arg(currentUserId))
		ereport(ERROR,
				(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
				 errmsg("must be superuser to set environment attribute")));

	if(strcmp(stmt->attribute, "workday") == 0)
		handle_workday(stmt);
	else if(strcmp(stmt->attribute, "timewindow") == 0)
		handle_timewindow(stmt);
	else if(strcmp(stmt->attribute, "subnet") == 0)
		handle_subnet(stmt);
	else
		ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_OBJECT),
				 errmsg("unrecognized environment attribute \"%s\"",
						stmt->attribute)));
}

void handle_workday(SetEnvAttributeStmt *stmt){
	Relation	pg_abac_env_workday_rel;
	TupleDesc	pg_abac_env_workday_dsc;
	ListCell   *item;
	const char *const days[] = {"sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"};
	bool is_workday[7] = {false, false, false, false, false, false, false};
	HeapTuple	tuple;  
	HeapTuple	newtuple;  
	Datum		values[Natts_pg_abac_env_workday];  
	bool		nulls[Natts_pg_abac_env_workday];  
	bool		replaces[Natts_pg_abac_env_workday];
	
	foreach(item, stmt->values){
		Node *node = (Node *) lfirst(item);
		char *day = strVal(lfirst(item));
		bool valid_day = false;
		int i;

		/* Extract string value from the node */  
		if (IsA(node, A_Const))  
		{  
			A_Const *con = (A_Const *) node;  
			day = strVal(&con->val);  
		}  
		else if (IsA(node, String))  
		{  
			day = strVal(node);  
		}  
		else  
		{  
			ereport(ERROR,  
					(errcode(ERRCODE_SYNTAX_ERROR),  
					errmsg("invalid value type for environment attribute")));  
		} 
		
		for(i = 0; i < 7; i++){
			if(strcmp(day, days[i]) == 0){
				valid_day = true;
				is_workday[i] = true;
				break;
			}
		}

		if(!valid_day){
			ereport(ERROR,
					(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
					 errmsg("invalid value \"%s\" for environment attribute \"%s\"",
							day, stmt->attribute),
					 errhint("Valid values are: sunday, monday, tuesday, wednesday, thursday, friday, saturday.")));
		}
	}

	pg_abac_env_workday_rel = table_open(AbacEnvWorkdayRelationId, RowExclusiveLock);
	pg_abac_env_workday_dsc = RelationGetDescr(pg_abac_env_workday_rel);

	/* Update each day of the week */  
	for(int i = 0; i < 7; i++){  
  
		tuple = SearchSysCache1(ABACENVWORKDAY, Int16GetDatum(i));  
		if (!HeapTupleIsValid(tuple))
			elog(ERROR, "cache lookup failed for day_of_week %d", i);

		memset(values, 0, sizeof(values));
		memset(nulls, false, sizeof(nulls));
		memset(replaces, false, sizeof(replaces));

		values[Anum_pg_abac_env_workday_is_workday - 1] = BoolGetDatum(is_workday[i]);  
		replaces[Anum_pg_abac_env_workday_is_workday - 1] = true;  
  
		newtuple = heap_modify_tuple(tuple, pg_abac_env_workday_dsc,  
									  values, nulls, replaces);  
		  
		CatalogTupleUpdate(pg_abac_env_workday_rel, &newtuple->t_self, newtuple);  
  
		heap_freetuple(newtuple);  
		ReleaseSysCache(tuple);  
	}

	table_close(pg_abac_env_workday_rel, NoLock);
}

void handle_timewindow(SetEnvAttributeStmt *stmt){
	int vals[4];
	int idx = 0;
	ListCell *lc;
	int sh, sm, eh, em;
	int start_min, end_min;
	Relation rel;
	TupleDesc dsc;
	HeapTuple tuple, newtuple;
	Datum values[Natts_pg_abac_env_timewindow];
	bool nulls[Natts_pg_abac_env_timewindow];
	bool replaces[Natts_pg_abac_env_timewindow];
	
	if (list_length(stmt->values) != 4)
    	ereport(ERROR,
			(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("timewindow requires 4 values: start_hour, start_minute, end_hour, end_minute")));


	foreach(lc, stmt->values)
	{
		Node *node = (Node *) lfirst(lc);

		if (!IsA(node, A_Const) || !IsA(&((A_Const *)node)->val, Integer))
			ereport(ERROR,
				(errcode(ERRCODE_SYNTAX_ERROR),
				errmsg("timewindow values must be integers")));

		vals[idx++] = intVal(&((A_Const *)node)->val);
	}

	sh = vals[0], sm = vals[1], eh = vals[2], em = vals[3];

	if (sh < 0 || sh > 23 || eh < 0 || eh > 23 ||
		sm < 0 || sm > 59 || em < 0 || em > 59)
		ereport(ERROR,
			(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("invalid hour/minute in timewindow")));

	start_min = sh * 60 + sm;
	end_min   = eh * 60 + em;

	if (start_min >= end_min)
		ereport(ERROR,
			(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("start time must be earlier than end time")));


	rel = table_open(AbacEnvTimewindowRelationId, RowExclusiveLock);
	dsc = RelationGetDescr(rel);

	tuple = SearchSysCache1(ABACENVTIMEWINDOW, ObjectIdGetDatum(1));
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "cache lookup failed for pg_abac_env_timewindow");

	memset(values, 0, sizeof(values));
	memset(nulls, false, sizeof(nulls));
	memset(replaces, false, sizeof(replaces));

	values[Anum_pg_abac_env_timewindow_start_minute - 1] = Int32GetDatum(start_min);
	values[Anum_pg_abac_env_timewindow_end_minute - 1]   = Int32GetDatum(end_min);
	replaces[Anum_pg_abac_env_timewindow_start_minute - 1] = true;
	replaces[Anum_pg_abac_env_timewindow_end_minute - 1]   = true;

	newtuple = heap_modify_tuple(tuple, dsc, values, nulls, replaces);
	CatalogTupleUpdate(rel, &newtuple->t_self, newtuple);

	heap_freetuple(newtuple);
	ReleaseSysCache(tuple);
	table_close(rel, NoLock);
}

void
handle_subnet(SetEnvAttributeStmt *stmt)
{
    Relation    rel;
    TupleDesc   dsc;
    ListCell   *lc;

    rel = table_open(AbacEnvSubnetRelationId, RowExclusiveLock);
    dsc = RelationGetDescr(rel);

    foreach(lc, stmt->values)
    {
        DefElem    *def;
        char       *subnet_name;
        char       *cidr_str;
        Datum       cidr_datum;
        HeapTuple   oldtuple;
        HeapTuple   newtuple;
        Datum       values[Natts_pg_abac_env_subnet];
        bool        nulls[Natts_pg_abac_env_subnet];
        bool        replaces[Natts_pg_abac_env_subnet];

        def = (DefElem *) lfirst(lc);
        subnet_name = def->defname;

        if (!IsA(def->arg, String))
            ereport(ERROR,
                (errcode(ERRCODE_SYNTAX_ERROR),
                 errmsg("subnet value must be a CIDR string")));

        cidr_str = strVal(def->arg);

        cidr_datum = DirectFunctionCall1(cidr_in,
                                         CStringGetDatum(cidr_str));

        oldtuple = SearchSysCache1(ABACENVSUBNET,
                                   CStringGetDatum(subnet_name));

        memset(values, 0, sizeof(values));
        memset(nulls, false, sizeof(nulls));
        memset(replaces, false, sizeof(replaces));

        values[Anum_pg_abac_env_subnet_network - 1] = cidr_datum;
        replaces[Anum_pg_abac_env_subnet_network - 1] = true;

        if (HeapTupleIsValid(oldtuple))
        {
            /* UPDATE existing subnet */
            newtuple = heap_modify_tuple(oldtuple,
                                         dsc,
                                         values,
                                         nulls,
                                         replaces);

            CatalogTupleUpdate(rel, &oldtuple->t_self, newtuple);

            heap_freetuple(newtuple);
            ReleaseSysCache(oldtuple);
        }
        else
        {
            /* INSERT new subnet */
            values[Anum_pg_abac_env_subnet_subnet_name - 1] =
                DirectFunctionCall1(namein,
                                    CStringGetDatum(subnet_name));

            newtuple = heap_form_tuple(dsc, values, nulls);
            CatalogTupleInsert(rel, newtuple);
            heap_freetuple(newtuple);
        }
    }

    table_close(rel, NoLock);
}

void
check_subnet_exists(const char *subnet_name)
{
    HeapTuple tuple;

    tuple = SearchSysCache1(ABACENVSUBNET,
                            CStringGetDatum(subnet_name));

    if (!HeapTupleIsValid(tuple))
        ereport(ERROR,
            (errcode(ERRCODE_UNDEFINED_OBJECT),
             errmsg("environment subnet \"%s\" does not exist",
                    subnet_name),
             errhint("Use SET ENV_ATTRIBUTE subnet to define it first.")));

    ReleaseSysCache(tuple);
}

void AddRuleAttr(Oid ruleid, Oid attrid, bool is_user_attr, const char* value)
{
	Relation	pg_abac_rule_rel;
	TupleDesc	pg_abac_rule_dsc;
	HeapTuple	tuple;
	Datum		new_record[Natts_pg_abac_rule] = {0};
	bool		new_record_nulls[Natts_pg_abac_rule] = {0};

	pg_abac_rule_rel = table_open(AbacRuleRelationId, RowExclusiveLock);
	pg_abac_rule_dsc = RelationGetDescr(pg_abac_rule_rel);

	new_record[Anum_pg_abac_rule_rule_id - 1] = ObjectIdGetDatum(ruleid);
	new_record[Anum_pg_abac_rule_attr_id - 1] = ObjectIdGetDatum(attrid);
	new_record[Anum_pg_abac_rule_is_user_attr - 1] = BoolGetDatum(is_user_attr);
	new_record[Anum_pg_abac_rule_value - 1] = CStringGetTextDatum(value);

	tuple = heap_form_tuple(pg_abac_rule_dsc, new_record, new_record_nulls);
	CatalogTupleInsert(pg_abac_rule_rel, tuple);

	heap_freetuple(tuple);
	table_close(pg_abac_rule_rel, NoLock);
}

AclMode
string_to_privilege(const char *privname){
	if (strcmp(privname, "insert") == 0)
		return ACL_INSERT;
	if (strcmp(privname, "select") == 0)
		return ACL_SELECT;
	if (strcmp(privname, "update") == 0)
		return ACL_UPDATE;
	if (strcmp(privname, "delete") == 0)
		return ACL_DELETE;
	if (strcmp(privname, "usage") == 0)
		return ACL_USAGE;
	if (strcmp(privname, "execute") == 0)
		return ACL_EXECUTE;
	ereport(ERROR,
			(errcode(ERRCODE_SYNTAX_ERROR),
			 errmsg("unrecognized privilege type \"%s\"", privname)));
	return 0;
}

void
CreateAbacRule(ParseState *pstate, CreateAbacRuleStmt *stmt)  
{  
	Relation	pg_abac_rule_priv_rel;
	TupleDesc	pg_abac_rule_priv_dsc;
	HeapTuple	tuple;  
	Datum		new_record[Natts_pg_abac_rule_priv] = {0};
	bool		new_record_nulls[Natts_pg_abac_rule_priv] = {0};
	List	   *user_attrs;    
	List	   *resource_attrs;    
	ListCell   *lc;
	Oid			rule_id;
	AclMode		privilege_mask = ACL_NO_RIGHTS;
	ListCell   *priv;
	AccessPriv *access_priv;
	Oid			currentUserId = GetUserId();
	float8		server_load = 0.0;
	Float	   *float_node;
	char	   *endptr;

	/* Only superusers can create ABAC rules */
	if (!superuser_arg(currentUserId))
		ereport(ERROR,
				(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
				 errmsg("must be superuser to create ABAC rule")));

	pg_abac_rule_priv_rel = table_open(AbacRulePrivRelationId, RowExclusiveLock);
	pg_abac_rule_priv_dsc = RelationGetDescr(pg_abac_rule_priv_rel);

	/* Check if rule with same name already exists */
	if (OidIsValid(get_abac_rule_oid(stmt->rule_name, true)))
		ereport(ERROR,
			(errcode(ERRCODE_DUPLICATE_OBJECT),
				errmsg("ABAC rule \"%s\" already exists",
					stmt->rule_name)));
	
	rule_id = GetNewOidWithIndex(pg_abac_rule_priv_rel, AbacRulePrivOidIndexId,
								Anum_pg_abac_rule_priv_oid);
	
	/* Compute privilege mask */
	if(stmt->privileges != NIL){
		foreach(priv, stmt->privileges){
			access_priv = (AccessPriv *) lfirst(priv);
			privilege_mask |= string_to_privilege(access_priv->priv_name);
		}
	}
	
	check_subnet_exists(stmt->subnet_name);

	if(stmt->server_load != NULL){
		if (!IsA(stmt->server_load, Float))  
        	elog(ERROR, "server_load is not a Float node");

		float_node = castNode(Float, stmt->server_load);
		server_load = strtod(float_node->fval, &endptr);

		if (endptr == float_node->fval || *endptr != '\0')
			ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("invalid server_load value \"%s\"", float_node->fval)));
		
		if(server_load < 0.0 || server_load > 1.0)
			ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
					errmsg("invalid server_load value %.2f, must be between 0.0 and 1.0", server_load)));
		
	}

	new_record[Anum_pg_abac_rule_priv_oid - 1] = ObjectIdGetDatum(rule_id);
	new_record[Anum_pg_abac_rule_priv_rulename - 1] = DirectFunctionCall1(namein, CStringGetDatum(stmt->rule_name));
	new_record[Anum_pg_abac_rule_priv_privileges - 1] = Int32GetDatum(privilege_mask);
	new_record[Anum_pg_abac_rule_priv_is_workday - 1] = BoolGetDatum(stmt->is_workday);
	new_record[Anum_pg_abac_rule_priv_is_worktime - 1] = BoolGetDatum(stmt->is_worktime);
	new_record[Anum_pg_abac_rule_priv_subnet_name - 1] = DirectFunctionCall1(namein, CStringGetDatum(stmt->subnet_name));
	new_record[Anum_pg_abac_rule_priv_server_load - 1] = Float8GetDatum(server_load);

	tuple = heap_form_tuple(pg_abac_rule_priv_dsc, new_record, new_record_nulls);    
	CatalogTupleInsert(pg_abac_rule_priv_rel, tuple);
	heap_freetuple(tuple);  
	
	/* Process user attributes */
	user_attrs = (List *) linitial(stmt->attribute_clause);
	if (user_attrs != NIL)  {  
		foreach(lc, user_attrs)  {  
			DefElem    *def = (DefElem *) lfirst(lc);  
			char	   *attr_name = def->defname;  
			char	   *attr_value = strVal(def->arg);  
			Oid			attr_id;  
			
			attr_id = get_user_attr_oid(attr_name, false);
			AddRuleAttr(rule_id, attr_id, true, attr_value);
		}
	}
	
	/* Process resource attributes */
	resource_attrs = (List *) lsecond(stmt->attribute_clause);    
	if (resource_attrs != NIL)  {
		foreach(lc, resource_attrs)  {
			DefElem    *def = (DefElem *) lfirst(lc);
			char	   *attr_name = def->defname;
			char	   *attr_value = strVal(def->arg);
			Oid			attr_id;  
			
			attr_id = get_resource_attr_oid(attr_name, false);  
			AddRuleAttr(rule_id, attr_id, false, attr_value);
		}  
	}

	table_close(pg_abac_rule_priv_rel, NoLock);
}

void DropAbacRule(ParseState *pstate, DropAbacRuleStmt *stmt){
	Relation    pg_abac_rule_rel;
    Relation    pg_abac_rule_priv_rel;
    ScanKeyData skey[1];
    SysScanDesc scan;
    HeapTuple   tuple;
    Oid         rule_priv_oid;
    Oid         currentUserId = GetUserId();
  
    /* Only superusers can drop ABAC rules */
    if (!superuser_arg(currentUserId))
        ereport(ERROR,
                (errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
                 errmsg("must be superuser to drop ABAC rule")));

    rule_priv_oid = get_abac_rule_oid(stmt->rule_name, stmt->missing_ok);

	if (!OidIsValid(rule_priv_oid) && stmt->missing_ok)
		return;

    pg_abac_rule_rel = table_open(AbacRuleRelationId, RowExclusiveLock);
    pg_abac_rule_priv_rel = table_open(AbacRulePrivRelationId, RowExclusiveLock);
  
    /* Delete all attribute entries from pg_abac_rule */
    ScanKeyInit(&skey[0],
                Anum_pg_abac_rule_rule_id,
                BTEqualStrategyNumber, F_OIDEQ,
                ObjectIdGetDatum(rule_priv_oid));
  
    scan = systable_beginscan(pg_abac_rule_rel, AbacRulePkeyIndexId, true,
                              NULL, 1, skey);
  
    while (HeapTupleIsValid(tuple = systable_getnext(scan)))
        CatalogTupleDelete(pg_abac_rule_rel, &tuple->t_self);
  
    systable_endscan(scan);
  
    /* Delete the privilege entry from pg_abac_rule_priv */
    ScanKeyInit(&skey[0],
                Anum_pg_abac_rule_priv_oid,
                BTEqualStrategyNumber, F_OIDEQ,
                ObjectIdGetDatum(rule_priv_oid));
  
    scan = systable_beginscan(pg_abac_rule_priv_rel, AbacRulePrivOidIndexId, true,
                              NULL, 1, skey);
  
    tuple = systable_getnext(scan);
    if (!HeapTupleIsValid(tuple))
        elog(ERROR, "could not find tuple for ABAC rule \"%s\"", stmt->rule_name);
  
    CatalogTupleDelete(pg_abac_rule_priv_rel, &tuple->t_self);
  
    systable_endscan(scan);
  
    table_close(pg_abac_rule_rel, RowExclusiveLock);
    table_close(pg_abac_rule_priv_rel, RowExclusiveLock);
  
    /* Advance command counter to make changes visible */
    CommandCounterIncrement();
}