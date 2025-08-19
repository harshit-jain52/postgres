/*-------------------------------------------------------------------------
 *
 * pg_resource_attr.h
 *	  definition of the "ABAC resource attributes" system catalog (pg_resource_attr)
 *
 *
 *
 * src/include/catalog/pg_resource_attr.h
 *
 * NOTES
 *	  The Catalog.pm module reads this file and derives schema
 *	  information.
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_RESOURCE_ATTR_H
#define PG_RESOURCE_ATTR_H

#include "catalog/genbki.h"
#include "catalog/pg_resource_attr_d.h"	/* IWYU pragma: export */

/* ----------------
 *		pg_resource_attr definition.  cpp turns this into
 *		typedef struct FormData_pg_resource_attr
 * ----------------
 */
CATALOG(pg_resource_attr,8755,ResourceAttrRelationId) BKI_SHARED_RELATION BKI_ROWTYPE_OID(8758,ResourceAttrRelation_Rowtype_Id) BKI_SCHEMA_MACRO
{
	Oid 		oid; 			/* oid */
	NameData	attrib_name;	/* name of attribute */
} FormData_pg_resource_attr;

/* ----------------
 *		Form_pg_resource_attr corresponds to a pointer to a tuple with
 *		the format of pg_resource_attr relation.
 * ----------------
 */
typedef FormData_pg_resource_attr *Form_pg_resource_attr;

DECLARE_UNIQUE_INDEX(pg_resource_attr_attrib_name_index, 8756, ResourceAttrAttribNameIndexId, pg_resource_attr, btree(attrib_name name_ops));
DECLARE_UNIQUE_INDEX_PKEY(pg_resource_attr_oid_index, 8757, ResourceAttrOidIndexId, pg_resource_attr, btree(oid oid_ops));

MAKE_SYSCACHE(RESOURCEATTRNAME, pg_resource_attr_attrib_name_index, 8);
MAKE_SYSCACHE(RESOURCEATTRID, pg_resource_attr_oid_index, 8);

#endif							/* PG_RESOURCE_ATTR_H */