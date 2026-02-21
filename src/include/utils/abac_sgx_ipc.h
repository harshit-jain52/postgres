#ifndef ABAC_SGX_IPC_H
#define ABAC_SGX_IPC_H

#include <stdint.h>

#define SGX_SOCKET_PATH "/tmp/sgx_abac.sock"

typedef struct {
    uint32_t type;
    uint32_t size;
} sgx_msg_hdr;

enum {
    SGX_MSG_SET_WORKDAY = 1,
    SGX_MSG_SET_TIMEWINDOW,
    SGX_MSG_SET_SUBNET,
    SGX_MSG_SUBNET_EXISTS,
    SGX_MSG_CHECK_ENV,
    SGX_MSG_DEBUG_DUMP,
    SGX_MSG_SHUTDOWN
};

typedef struct {
    uint8_t workday[7];
} msg_set_workday;

typedef struct {
    int start_min;
    int end_min;
} msg_set_timewindow;

typedef struct {
    char name[64];
    uint32_t network;
    uint32_t mask;
} msg_set_subnet;

typedef struct {
    char name[64];
} msg_subnet_exists;

typedef struct {
    int day_of_week;
    int current_minute;
    uint32_t client_ip;
    char subnet_name[64];

    int workday_value;
    int worktime_value;

    int check_workday;
    int check_worktime;
    int check_subnet;
} msg_check_env;

#endif /* ABAC_SGX_IPC_H */