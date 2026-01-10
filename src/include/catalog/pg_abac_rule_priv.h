/*-------------------------------------------------------------------------  
 *  
 * pg_abac_rule_priv.h  
 *	  definition of the "ABAC rule privileges" system catalog (pg_abac_rule_priv)  
 *  
 * src/include/catalog/pg_abac_rule_priv.h  
 *  
 * NOTES  
 *	  The Catalog.pm module reads this file and derives schema  
 *	  information.  
 *  
 *-------------------------------------------------------------------------  
 */  
#ifndef PG_ABAC_RULE_PRIV_H  
#define PG_ABAC_RULE_PRIV_H  
  
#include "catalog/genbki.h"  
#include "catalog/pg_abac_rule_priv_d.h"	/* IWYU pragma: export */  
  
/* ----------------  
 *		pg_abac_rule_priv definition.  cpp turns this into  
 *		typedef struct FormData_pg_abac_rule_priv  
 * ----------------  
 */  
CATALOG(pg_abac_rule_priv,8790,AbacRulePrivRelationId) BKI_SHARED_RELATION BKI_ROWTYPE_OID(8793,AbacRulePrivRelation_Rowtype_Id) BKI_SCHEMA_MACRO  
{  
	Oid			oid;			/* oid */  
	NameData	rulename;		/* Rulename */  
	int32		privileges;		/* bitmask of privileges (AclMode) */
	bool		is_workday;		/* value of the environment attribute condition */
	bool		is_worktime;	/* value of the environment attribute condition */
	NameData	subnet_name;
	float8		server_load;
} FormData_pg_abac_rule_priv;  
  
/* ----------------  
 *		Form_pg_abac_rule_priv corresponds to a pointer to a tuple with  
 *		the format of pg_abac_rule_priv relation.  
 * ----------------  
 */  
typedef FormData_pg_abac_rule_priv *Form_pg_abac_rule_priv;  
  
DECLARE_UNIQUE_INDEX_PKEY(pg_abac_rule_priv_oid_index, 8791, AbacRulePrivOidIndexId, pg_abac_rule_priv, btree(oid oid_ops));  
DECLARE_UNIQUE_INDEX(pg_abac_rule_priv_rulename_index, 8792, AbacRulePrivRulenameIndexId, pg_abac_rule_priv, btree(rulename name_ops));  
  
MAKE_SYSCACHE(ABACRULEPRIVRULENAME, pg_abac_rule_priv_rulename_index, 8);  
MAKE_SYSCACHE(ABACRULEPRIV, pg_abac_rule_priv_oid_index, 8);  
  
#endif							/* PG_ABAC_RULE_PRIV_H */