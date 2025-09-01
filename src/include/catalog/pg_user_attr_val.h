/*-------------------------------------------------------------------------
 *
 * pg_user_attr_val.h
 *	  definition of the "ABAC user attribute values" system catalog (pg_user_attr_val)
 *
 * src/include/catalog/pg_user_attr_val.h
 *
 * NOTES
 *	  The Catalog.pm module reads this file and derives schema
 *	  information.
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_USER_ATTR_VAL_H
#define PG_USER_ATTR_VAL_H

#include "catalog/genbki.h"
#include "catalog/pg_user_attr_val_d.h" /* IWYU pragma: export */

/* ----------------
 *		pg_user_attr_val definition.  cpp turns this into
 *		typedef struct FormData_pg_user_attr_val
 * ----------------
 */
CATALOG(pg_user_attr_val, 8770, UserAttrValRelationId) BKI_SHARED_RELATION BKI_ROWTYPE_OID(8773, UserAttrValRelation_Rowtype_Id) BKI_SCHEMA_MACRO
{
    Oid user_id BKI_LOOKUP(pg_authid);      /* foreign key to oid in pg_authid */  
    Oid attr_id BKI_LOOKUP(pg_user_attr);   /* foreign key to oid in pg_user_attr */

    /* remaining fields may be null; use heap_getattr to read them! */
#ifdef CATALOG_VARLEN /* variable-length fields start here */
    text value;       /* attribute value */
#endif
}
FormData_pg_user_attr_val;

/* ----------------
 *		Form_pg_user_attr_val corresponds to a pointer to a tuple with
 *		the format of pg_user_attr_val relation.
 * ----------------
 */
typedef FormData_pg_user_attr_val *Form_pg_user_attr_val;

DECLARE_UNIQUE_INDEX_PKEY(pg_user_attr_val_pkey, 8771, UserAttrValPkeyIndexId, pg_user_attr_val, btree(user_id oid_ops, attr_id oid_ops));

MAKE_SYSCACHE(USERATTRVALUSERATTR, pg_user_attr_val_pkey, 8);

#endif /* PG_USER_ATTR_VAL_H */