/*-------------------------------------------------------------------------  
 *  
 * pg_abac_env_subnet.h  
 *	  definition of the "ABAC environment attribute subnet" system catalog (pg_abac_env_subnet)  
 *  
 * src/include/catalog/pg_abac_env_subnet.h  
 *  
 * NOTES  
 *	  The Catalog.pm module reads this file and derives schema  
 *	  information.  
 *  
 *-------------------------------------------------------------------------  
 */  
#ifndef PG_ABAC_ENV_SUBNET_H  
#define PG_ABAC_ENV_SUBNET_H  
  
#include "catalog/genbki.h"
#include "utils/inet.h"
#include "catalog/pg_abac_env_subnet_d.h"	/* IWYU pragma: export */  
  
/* ----------------  
 *		pg_abac_env_subnet definition.  cpp turns this into  
 *		typedef struct FormData_pg_abac_env_subnet  
 * ----------------  
 */  
CATALOG(pg_abac_env_subnet,8996,AbacEnvSubnetRelationId) BKI_SHARED_RELATION BKI_ROWTYPE_OID(8998,AbacEnvSubnetRelation_Rowtype_Id) BKI_SCHEMA_MACRO  
{  
	NameData subnet_name;   /* name of subnet */
    inet    network;        /* CIDR network */
} FormData_pg_abac_env_subnet;  
  
/* ----------------  
 *		Form_pg_abac_env_subnet corresponds to a pointer to a tuple with  
 *		the format of pg_abac_env_subnet relation.  
 * ----------------  
 */  
typedef FormData_pg_abac_env_subnet *Form_pg_abac_env_subnet;

DECLARE_UNIQUE_INDEX_PKEY(pg_abac_env_subnet_name_index, 8997, AbacEnvSubnetNameIndexId, pg_abac_env_subnet, btree(subnet_name name_ops));

MAKE_SYSCACHE(ABACENVSUBNET, pg_abac_env_subnet_name_index, 8);

#endif							/* PG_ABAC_ENV_SUBNET_H */