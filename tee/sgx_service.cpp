#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include "sgx_urts.h"
#include "Enclave_u.h"

#define ENCLAVE_PATH "enclave.signed.so"
#define SEALED_FILE  "sealed_env.bin"
#define SOCKET_PATH  "/tmp/sgx_abac.sock"

sgx_enclave_id_t global_eid = 0;
uint8_t *sealed_buf = NULL;
uint32_t sealed_size = 0;

/* ---------- Protocol ---------- */

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

/* ---------- Helpers ---------- */

void fatal(const char* msg)
{
    perror(msg);
    exit(1);
}

void check_sgx(sgx_status_t ret, const char* msg)
{
    if (ret != SGX_SUCCESS)
    {
        printf("%s failed: 0x%x\n", msg, ret);
        exit(1);
    }
}

void ocall_print(const char* str)
{
    printf("[ENCLAVE] %s\n", str);
}

/* ---------- Sealing Logic ---------- */

void reseal_env()
{
    sgx_status_t ret, ecall_ret;

    ret = enclave_seal_env(global_eid, &ecall_ret,
                           (sgx_sealed_data_t*)sealed_buf,
                           sealed_size);
    check_sgx(ret, "seal transport");
    check_sgx(ecall_ret, "seal enclave");

    FILE* fp = fopen(SEALED_FILE, "wb");
    if (!fp) fatal("fopen seal");

    fwrite(sealed_buf, 1, sealed_size, fp);
    fclose(fp);
}

void load_or_init_env()
{
    sgx_status_t ret, ecall_ret;

    ret = enclave_get_sealed_size(global_eid, &ecall_ret, &sealed_size);
    check_sgx(ret, "get sealed size");
    check_sgx(ecall_ret, "get sealed size enclave");

    sealed_buf = (uint8_t*)malloc(sealed_size);

    FILE* fp = fopen(SEALED_FILE, "rb");

    if (fp)
    {
        fread(sealed_buf, 1, sealed_size, fp);
        fclose(fp);

        ret = enclave_unseal_env(global_eid, &ecall_ret,
                                 (sgx_sealed_data_t*)sealed_buf,
                                 sealed_size);
        check_sgx(ret, "unseal transport");
        check_sgx(ecall_ret, "unseal enclave");

        printf("ABAC environment unsealed successfully\n");
    }
    else
    {
        ret = enclave_init_env(global_eid, &ecall_ret,
                               (sgx_sealed_data_t*)sealed_buf,
                               sealed_size);
        check_sgx(ret, "init transport");
        check_sgx(ecall_ret, "init enclave");

        reseal_env();
        printf("ABAC environment initialized and sealed\n");
    }
}

/* ---------- Main ---------- */

int main()
{
    sgx_status_t ret;

    printf("Starting SGX ABAC Service...\n");

    ret = sgx_create_enclave(ENCLAVE_PATH,
                             SGX_DEBUG_FLAG,
                             NULL,
                             NULL,
                             &global_eid,
                             NULL);
    check_sgx(ret, "sgx_create_enclave");

    load_or_init_env();
    enclave_debug_dump_env(global_eid, &ret);

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) fatal("socket");

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    unlink(SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        fatal("bind");
    
    struct group *grp = getgrnam("sgxabac");

    if (!grp) {
        fatal("getgrnam");
    }

    if (chmod(SOCKET_PATH, 0660) != 0)
        perror("chmod");

    if (chown(SOCKET_PATH, -1, grp->gr_gid) != 0)
        perror("chown");
        
    if (listen(server_fd, 10) < 0)
        fatal("listen");

    printf("SGX service listening on %s\n", SOCKET_PATH);

    while (1)
    {
        int client = accept(server_fd, NULL, NULL);
        if (client < 0)
            continue;

        struct ucred cred;
        socklen_t len = sizeof(cred);

        if (getsockopt(client, SOL_SOCKET, SO_PEERCRED, &cred, &len) == -1){
            perror("getsockopt");
            close(client);
            continue;
        }

        if (cred.uid != getpwnam("postgres")->pw_uid)
        {
            printf("Unauthorized client uid=%d\n", cred.uid);
            close(client);
            continue;
        }

        sgx_msg_hdr hdr;
        if (read(client, &hdr, sizeof(hdr)) != sizeof(hdr))
        {
            close(client);
            continue;
        }

        sgx_status_t ecall_ret;

        switch (hdr.type)
        {
            case SGX_MSG_SET_WORKDAY:
            {
                msg_set_workday payload;
                read(client, &payload, sizeof(payload));

                enclave_set_workday(global_eid, &ecall_ret,
                                    payload.workday);
                reseal_env();
                break;
            }

            case SGX_MSG_SET_TIMEWINDOW:
            {
                msg_set_timewindow payload;
                read(client, &payload, sizeof(payload));

                enclave_set_timewindow(global_eid, &ecall_ret,
                                       payload.start_min,
                                       payload.end_min);
                reseal_env();
                break;
            }

            case SGX_MSG_SET_SUBNET:
            {
                msg_set_subnet payload;
                read(client, &payload, sizeof(payload));

                enclave_set_subnet(global_eid, &ecall_ret,
                                   payload.name,
                                   payload.network,
                                   payload.mask);
                reseal_env();
                break;
            }

            case SGX_MSG_SUBNET_EXISTS:
            {
                msg_subnet_exists payload;
                int exists;

                read(client, &payload, sizeof(payload));

                enclave_subnet_exists(global_eid, &ecall_ret,
                                      payload.name,
                                      &exists);

                write(client, &exists, sizeof(exists));
                break;
            }

            case SGX_MSG_CHECK_ENV:
            {
                msg_check_env payload;
                int result;

                read(client, &payload, sizeof(payload));

                enclave_check_env(global_eid, &ecall_ret,
                                  payload.day_of_week,
                                  payload.current_minute,
                                  payload.client_ip,
                                  payload.subnet_name,
                                  payload.workday_value,
                                  payload.worktime_value,
                                  payload.check_workday,
                                  payload.check_worktime,
                                  payload.check_subnet,
                                  &result);

                write(client, &result, sizeof(result));
                break;
            }

            case SGX_MSG_DEBUG_DUMP:
            {
                enclave_debug_dump_env(global_eid, &ecall_ret);
                break;
            }

            case SGX_MSG_SHUTDOWN:
            {
                printf("Shutting down SGX service...\n");
                close(client);
                sgx_destroy_enclave(global_eid);
                unlink(SOCKET_PATH);
                return 0;
            }

            default:
                break;
        }

        close(client);
    }

    return 0;
}