/*-------------------------------------------------------------------------  
 *  
 * pg_abac_rule.h  
 *	  definition of the "ABAC rules" system catalog (pg_abac_rule)  
 *  
 * src/include/catalog/pg_abac_rule.h  
 *  
 * NOTES  
 *	  The Catalog.pm module reads this file and derives schema  
 *	  information.  
 *  
 *-------------------------------------------------------------------------  
 */  
#ifndef PG_ABAC_RULE_H  
#define PG_ABAC_RULE_H  
  
#include "catalog/genbki.h"  
#include "catalog/pg_abac_rule_d.h"	/* IWYU pragma: export */  
  
/* ----------------  
 *		pg_abac_rule definition.  cpp turns this into  
 *		typedef struct FormData_pg_abac_rule  
 * ----------------  
 */  
CATALOG(pg_abac_rule,8780,AbacRuleRelationId) BKI_SHARED_RELATION BKI_ROWTYPE_OID(8783,AbacRuleRelation_Rowtype_Id) BKI_SCHEMA_MACRO  
{  
	NameData	rulename;		/* name of the ABAC rule */  
	Oid			attr_id;		/* either pg_user_attr.oid or pg_resource_attr.oid */  
	bool		is_user_attr;	/* true if attr_id references pg_user_attr, false if pg_resource_attr */  
  
	/* remaining fields may be null; use heap_getattr to read them! */  
#ifdef CATALOG_VARLEN			/* variable-length fields start here */  
	text		value;			/* attribute value for the rule */  
#endif  
} FormData_pg_abac_rule; 
  
/* ----------------  
 *		Form_pg_abac_rule corresponds to a pointer to a tuple with  
 *		the format of pg_abac_rule relation.  
 * ----------------  
 */  
typedef FormData_pg_abac_rule *Form_pg_abac_rule;  
  
DECLARE_UNIQUE_INDEX_PKEY(pg_abac_rule_pkey, 8781, AbacRulePkeyIndexId, pg_abac_rule, btree(rulename name_ops, attr_id oid_ops));  
  
MAKE_SYSCACHE(ABACRULENAMEATTR, pg_abac_rule_pkey, 8);  
  
#endif							/* PG_ABAC_RULE_H */