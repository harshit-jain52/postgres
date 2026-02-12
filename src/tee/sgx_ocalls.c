#include "postgres.h"
#include "utils/elog.h"

void ocall_print(const char *str)
{
    elog(LOG, "SGX: %s", str);
}
