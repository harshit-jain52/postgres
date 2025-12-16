/*-------------------------------------------------------------------------  
 *  
 * pg_abac_env_timewindow.h  
 *	  definition of the "ABAC environment attribute timewindow" system catalog (pg_abac_env_timewindow)  
 *  
 * src/include/catalog/pg_abac_env_timewindow.h  
 *  
 * NOTES  
 *	  The Catalog.pm module reads this file and derives schema  
 *	  information.  
 *  
 *-------------------------------------------------------------------------  
 */  
#ifndef PG_ABAC_ENV_TIMEWINDOW_H  
#define PG_ABAC_ENV_TIMEWINDOW_H  
  
#include "catalog/genbki.h"  
#include "catalog/pg_abac_env_timewindow_d.h"	/* IWYU pragma: export */  
  
/* ----------------  
 *		pg_abac_env_timewindow definition.  cpp turns this into  
 *		typedef struct FormData_pg_abac_env_timewindow  
 * ----------------  
 */  
CATALOG(pg_abac_env_timewindow,8993,AbacEnvTimewindowRelationId) BKI_SHARED_RELATION BKI_ROWTYPE_OID(8995,AbacEnvTimewindowRelation_Rowtype_Id) BKI_SCHEMA_MACRO  
{  
	int16       id;             /* Dummy constant key (=1) needed for syscache */ 
    int16       start_minute;   /* Start time in minutes from midnight (0-1439) */
    int16       end_minute;     /* End time in minutes from midnight (0-1439) */
} FormData_pg_abac_env_timewindow;  
  
/* ----------------  
 *		Form_pg_abac_env_timewindow corresponds to a pointer to a tuple with  
 *		the format of pg_abac_env_timewindow relation.  
 * ----------------  
 */  
typedef FormData_pg_abac_env_timewindow *Form_pg_abac_env_timewindow;

/*
 * Single-row catalog: enforce exactly one tuple
 * We use a dummy constant key (=1) for syscache access
 */
DECLARE_UNIQUE_INDEX_PKEY(pg_abac_env_timewindow_pkey, 8994, AbacEnvTimeWindowPkeyIndexId, pg_abac_env_timewindow, btree(id int2_ops));

MAKE_SYSCACHE(ABACENVTIMEWINDOW, pg_abac_env_timewindow_pkey, 8);

#endif							/* PG_ABAC_ENV_TIMEWINDOW_H */