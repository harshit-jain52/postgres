#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

#define SOCKET_PATH "/tmp/sgx_abac.sock"

typedef struct {
    unsigned int type;
    unsigned int size;
} sgx_msg_hdr;

int main()
{
    int sock;
    struct sockaddr_un addr;

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        return 1;
    }

    sgx_msg_hdr hdr;
    hdr.type = 6;   // SGX_MSG_DEBUG_DUMP
    hdr.size = 0;

    write(sock, &hdr, sizeof(hdr));

    printf("Message sent\n");

    close(sock);
}