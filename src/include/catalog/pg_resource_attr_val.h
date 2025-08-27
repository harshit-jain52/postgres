/*-------------------------------------------------------------------------
 *
 * pg_resource_attr_val.h
 *	  definition of the "ABAC resource attribute values" system catalog (pg_resource_attr_val)
 *
 * src/include/catalog/pg_resource_attr_val.h
 *
 * NOTES
 *	  The Catalog.pm module reads this file and derives schema
 *	  information.
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_RESOURCE_ATTR_VAL_H
#define PG_RESOURCE_ATTR_VAL_H

#include "catalog/genbki.h"
#include "catalog/pg_resource_attr_val_d.h" /* IWYU pragma: export */

/* ----------------
 *		pg_resource_attr_val definition.  cpp turns this into
 *		typedef struct FormData_pg_resource_attr_val
 * ----------------
 */
CATALOG(pg_resource_attr_val, 8760, ResourceAttrValRelationId) BKI_SHARED_RELATION BKI_ROWTYPE_OID(8763, ResourceAttrValRelation_Rowtype_Id) BKI_SCHEMA_MACRO
{
    NameData resource_name BKI_LOOKUP(pg_class);    /* foreign key to relname in pg_class */
    NameData attribute BKI_LOOKUP(pg_resource_attr); /* foreign key to attrib_name in pg_resource_attr */

    /* remaining fields may be null; use heap_getattr to read them! */
#ifdef CATALOG_VARLEN /* variable-length fields start here */
    text value;       /* attribute value */
#endif
}
FormData_pg_resource_attr_val;

/* ----------------
 *		Form_pg_resource_attr_val corresponds to a pointer to a tuple with
 *		the format of pg_resource_attr_val relation.
 * ----------------
 */
typedef FormData_pg_resource_attr_val *Form_pg_resource_attr_val;

DECLARE_UNIQUE_INDEX_PKEY(pg_resource_attr_val_pkey, 8761, ResourceAttrValPkeyIndexId, pg_resource_attr_val, btree(resource_name name_ops, attribute name_ops));

MAKE_SYSCACHE(RESOURCEATTRVALRESOURCEATTR, pg_resource_attr_val_pkey, 8);

#endif /* PG_RESOURCE_ATTR_VAL_H */