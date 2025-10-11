/*-------------------------------------------------------------------------  
 *  
 * pg_abac_env_workday.h  
 *	  definition of the "ABAC environment attribute workday" system catalog (pg_abac_env_workday)  
 *  
 * src/include/catalog/pg_abac_env_workday.h  
 *  
 * NOTES  
 *	  The Catalog.pm module reads this file and derives schema  
 *	  information.  
 *  
 *-------------------------------------------------------------------------  
 */  
#ifndef PG_ABAC_ENV_WORKDAY_H  
#define PG_ABAC_ENV_WORKDAY_H  
  
#include "catalog/genbki.h"  
#include "catalog/pg_abac_env_workday_d.h"	/* IWYU pragma: export */  
  
/* ----------------  
 *		pg_abac_env_workday definition.  cpp turns this into  
 *		typedef struct FormData_pg_abac_env_workday  
 * ----------------  
 */  
CATALOG(pg_abac_env_workday,8990,AbacEnvWorkdayRelationId) BKI_SHARED_RELATION BKI_ROWTYPE_OID(8992,AbacEnvWorkdayRelation_Rowtype_Id) BKI_SCHEMA_MACRO  
{  
	int16       day_of_week;     /* 0=Sunday, 1=Monday, etc. */  
    bool        is_workday;      /* true if it's a workday, false otherwise */ 
} FormData_pg_abac_env_workday;  
  
/* ----------------  
 *		Form_pg_abac_env_workday corresponds to a pointer to a tuple with  
 *		the format of pg_abac_env_workday relation.  
 * ----------------  
 */  
typedef FormData_pg_abac_env_workday *Form_pg_abac_env_workday;

DECLARE_UNIQUE_INDEX_PKEY(pg_abac_env_workday_day_of_week_index, 8991, AbacEnvWorkdayDayOfWeekIndexId, pg_abac_env_workday, btree(day_of_week int2_ops));

MAKE_SYSCACHE(ABACENVWORKDAY, pg_abac_env_workday_day_of_week_index, 8);

#endif							/* PG_ABAC_ENV_WORKDAY_H */