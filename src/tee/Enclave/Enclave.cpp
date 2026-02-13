#include "Enclave_t.h"
#include "sgx_tseal.h"
#include <cstring>

typedef struct _env_config_t {
    uint8_t workday[7];
    int start_minute;
    int end_minute;

    int subnet_count;
    struct {
        char name[64];
        uint32_t network;
        uint32_t mask;
    } subnets[32];

} env_config_t;

static env_config_t g_env_config;

sgx_status_t enclave_init_env(sgx_sealed_data_t* sealed_data,
                              uint32_t sealed_size)
{
    memset(&g_env_config, 0, sizeof(g_env_config));
    for (int i = 1; i <= 5; i++)
        g_env_config.workday[i] = 1;
    g_env_config.workday[0] = 0;
    g_env_config.workday[6] = 0;
    g_env_config.start_minute = 0;
    g_env_config.end_minute = 24 * 60;
    g_env_config.subnet_count = 0;
    return SGX_SUCCESS;
}

sgx_status_t enclave_set_workday(uint8_t* workday_arr)
{
    memcpy(g_env_config.workday, workday_arr, 7);
    return SGX_SUCCESS;
}

sgx_status_t enclave_set_timewindow(int start_min, int end_min)
{
    g_env_config.start_minute = start_min;
    g_env_config.end_minute   = end_min;
    return SGX_SUCCESS;
}

sgx_status_t enclave_set_subnet(const char* name,
                                uint32_t network,
                                uint32_t mask)
{
    if (g_env_config.subnet_count >= 32)
        return SGX_ERROR_OUT_OF_MEMORY;

    int idx = g_env_config.subnet_count++;

    strncpy(g_env_config.subnets[idx].name, name, 63);
    g_env_config.subnets[idx].network = network;
    g_env_config.subnets[idx].mask = mask;

    return SGX_SUCCESS;
}

sgx_status_t enclave_seal_env(sgx_sealed_data_t* sealed_data,
                              uint32_t sealed_size)
{
    uint32_t required_size =
        sgx_calc_sealed_data_size(0, sizeof(env_config_t));

    if (sealed_size < required_size)
        return SGX_ERROR_INVALID_PARAMETER;

    return sgx_seal_data(0,
                         NULL,
                         sizeof(env_config_t),
                         (uint8_t*)&g_env_config,
                         sealed_size,
                         sealed_data);
}

sgx_status_t enclave_unseal_env(const sgx_sealed_data_t* sealed_data,
                                uint32_t sealed_size)
{
    uint32_t mac_len = 0;
    uint32_t decrypt_len = sizeof(env_config_t);

    return sgx_unseal_data(sealed_data,
                           NULL,
                           &mac_len,
                           (uint8_t*)&g_env_config,
                           &decrypt_len);
}

sgx_status_t enclave_get_sealed_size(uint32_t* sealed_size)
{
    *sealed_size =
        sgx_calc_sealed_data_size(0, sizeof(env_config_t));
    return SGX_SUCCESS;
}

sgx_status_t enclave_subnet_exists(const char* name,
                                   int* exists)
{
    *exists = 0;

    for (int i = 0; i < g_env_config.subnet_count; i++)
    {
        if (strncmp(g_env_config.subnets[i].name,
                    name,
                    64) == 0)
        {
            *exists = 1;
            return SGX_SUCCESS;
        }
    }

    return SGX_SUCCESS;
}

sgx_status_t enclave_check_env(int day_of_week,
                               int current_minute,
                               uint32_t client_ip,
                               const char* subnet_name,
                               int workday_value,
                               int worktime_value,
                               int check_workday,
                               int check_worktime,
                               int check_subnet,
                               int* result)
{
    *result = 1;

    if (check_workday)
    {
        if (workday_value != g_env_config.workday[day_of_week])
        {
            *result = 0;
            return SGX_SUCCESS;
        }
    }

    if (check_worktime)
    {   
        if(!(worktime_value == (current_minute >= g_env_config.start_minute && current_minute <= g_env_config.end_minute)))
        {
            *result = 0;
            return SGX_SUCCESS;
        }
    }

    if (check_subnet)
    {
        bool matched = false;

        for (int i = 0; i < g_env_config.subnet_count; i++)
        {
            if (strncmp(g_env_config.subnets[i].name,
                        subnet_name,
                        64) == 0)
            {
                uint32_t mask =
                    (g_env_config.subnets[i].mask == 0)
                        ? 0
                        : 0xFFFFFFFF << (32 - g_env_config.subnets[i].mask);

                if ((client_ip & mask) ==
                    (g_env_config.subnets[i].network & mask))
                {
                    matched = true;
                }
                break;
            }
        }

        if (!matched)
        {
            *result = 0;
            return SGX_SUCCESS;
        }
    }

    return SGX_SUCCESS;
}

sgx_status_t enclave_debug_get_workday(uint8_t* arr)
{
    memcpy(arr, g_env_config.workday, 7);
    return SGX_SUCCESS;
}

sgx_status_t enclave_debug_get_timewindow(int* start_min,
                                          int* end_min)
{
    *start_min = g_env_config.start_minute;
    *end_min   = g_env_config.end_minute;
    return SGX_SUCCESS;
}

sgx_status_t enclave_debug_get_subnet_count(int* count)
{
    *count = g_env_config.subnet_count;
    return SGX_SUCCESS;
}
