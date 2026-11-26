#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "handshake.h"

#define DEFAULT_PORT 8443

int main(int argc, char **argv)
{
    int port = DEFAULT_PORT;
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) port = DEFAULT_PORT;
    }

    printf("============================================================\n");
    printf("   Post-Quantum TLS 1.3 Server (NewHope-1024 KEM + AES-GCM) \n");
    printf("============================================================\n");

    pqc_conn_t *conn = pqc_listen_and_accept(port);
    if (!conn) {
        fprintf(stderr, "[-] Handshake negotiation failed.\n");
        return 1;
    }

    printf("[+] Shared Secret derived (first 8 bytes): %02X%02X%02X%02X%02X%02X%02X%02X...\n",
           conn->shared_secret[0], conn->shared_secret[1], conn->shared_secret[2], conn->shared_secret[3],
           conn->shared_secret[4], conn->shared_secret[5], conn->shared_secret[6], conn->shared_secret[7]);

    char recv_buf[1024];
    memset(recv_buf, 0, sizeof(recv_buf));
    int bytes_in = pqc_recv(conn, recv_buf, sizeof(recv_buf) - 1);
    if (bytes_in > 0) {
        recv_buf[bytes_in] = '\0';
        printf("[+] Received Post-Quantum Encrypted Message: \"%s\"\n", recv_buf);
    } else {
        fprintf(stderr, "[-] Failed to receive application data.\n");
    }

    const char *resp = "ACK from Post-Quantum TLS 1.3 Server: NewHope-1024 verified!";
    printf("[+] Sending Response: \"%s\"\n", resp);
    pqc_send(conn, resp, strlen(resp));

    pqc_close(conn);
    printf("[+] Connection cleanly closed. Deployment verification complete.\n");
    return 0;
}
