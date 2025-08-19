/*-------------------------------------------------------------------------
 *
 * pg_user_attr.h
 *	  definition of the "ABAC user attributes" system catalog (pg_user_attr)
 *
 *
 *
 * src/include/catalog/pg_user_attr.h
 *
 * NOTES
 *	  The Catalog.pm module reads this file and derives schema
 *	  information.
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_USER_ATTR_H
#define PG_USER_ATTR_H

#include "catalog/genbki.h"
#include "catalog/pg_user_attr_d.h"	/* IWYU pragma: export */

/* ----------------
 *		pg_user_attr definition.  cpp turns this into
 *		typedef struct FormData_pg_user_attr
 * ----------------
 */
CATALOG(pg_user_attr,8765,UserAttrRelationId) BKI_SHARED_RELATION BKI_ROWTYPE_OID(8768,UserAttrRelation_Rowtype_Id) BKI_SCHEMA_MACRO
{
	Oid 		oid; 			/* oid */
	NameData	attrib_name;	/* name of attribute */
} FormData_pg_user_attr;

/* ----------------
 *		Form_pg_user_attr corresponds to a pointer to a tuple with
 *		the format of pg_user_attr relation.
 * ----------------
 */
typedef FormData_pg_user_attr *Form_pg_user_attr;

DECLARE_UNIQUE_INDEX(pg_user_attr_attrib_name_index, 8766, UserAttrAttribNameIndexId, pg_user_attr, btree(attrib_name name_ops));
DECLARE_UNIQUE_INDEX_PKEY(pg_user_attr_oid_index, 8767, UserAttrOidIndexId, pg_user_attr, btree(oid oid_ops));

MAKE_SYSCACHE(USERATTRNAME, pg_user_attr_attrib_name_index, 8);
MAKE_SYSCACHE(USERATTRID, pg_user_attr_oid_index, 8);

#endif							/* PG_USER_ATTR_H */