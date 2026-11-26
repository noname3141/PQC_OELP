#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "handshake.h"

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 8443

int main(int argc, char **argv)
{
    const char *host = DEFAULT_HOST;
    int port = DEFAULT_PORT;

    if (argc > 1) host = argv[1];
    if (argc > 2) {
        port = atoi(argv[2]);
        if (port <= 0 || port > 65535) port = DEFAULT_PORT;
    }

    printf("============================================================\n");
    printf("   Post-Quantum TLS 1.3 Client (NewHope-1024 KEM + AES-GCM) \n");
    printf("============================================================\n");
    printf("[*] Connecting to %s:%d...\n", host, port);

    pqc_conn_t *conn = pqc_connect(host, port);
    if (!conn) {
        fprintf(stderr, "[-] Handshake negotiation failed.\n");
        return 1;
    }

    printf("[+] Shared Secret derived (first 8 bytes): %02X%02X%02X%02X%02X%02X%02X%02X...\n",
           conn->shared_secret[0], conn->shared_secret[1], conn->shared_secret[2], conn->shared_secret[3],
           conn->shared_secret[4], conn->shared_secret[5], conn->shared_secret[6], conn->shared_secret[7]);

    const char *msg = "Hello from Post-Quantum Client! NewHope-1024 + TLS 1.3 operational.";
    printf("[+] Sending Post-Quantum Encrypted Message: \"%s\"\n", msg);
    if (pqc_send(conn, msg, strlen(msg)) < 0) {
        fprintf(stderr, "[-] Failed to send application data.\n");
        pqc_close(conn);
        return 1;
    }

    char recv_buf[1024];
    memset(recv_buf, 0, sizeof(recv_buf));
    int bytes_in = pqc_recv(conn, recv_buf, sizeof(recv_buf) - 1);
    if (bytes_in > 0) {
        recv_buf[bytes_in] = '\0';
        printf("[+] Received Server Response: \"%s\"\n", recv_buf);
    } else {
        fprintf(stderr, "[-] Failed to receive server response.\n");
    }

    pqc_close(conn);
    printf("[+] Connection cleanly closed. Post-quantum session completed.\n");
    return 0;
}
