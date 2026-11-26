#ifndef HANDSHAKE_H
#define HANDSHAKE_H

#include <stddef.h>
#include <stdint.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "newhope.h"

#define MSG_TYPE_PUBLIC_KEY  0x01
#define MSG_TYPE_CIPHERTEXT  0x02
#define MSG_TYPE_SUCCESS     0x03
#define MSG_TYPE_APP_DATA    0x04

#define AES_GCM_IV_LEN       12
#define AES_GCM_TAG_LEN      16
#define AES_GCM_KEY_LEN      32

#pragma pack(push, 1)
typedef struct {
    uint8_t  type;
    uint32_t length;
} frame_header_t;
#pragma pack(pop)

typedef struct {
    int socket_fd;
    SSL *ssl;
    SSL_CTX *ctx;
    uint8_t shared_secret[CRYPTO_BYTES];
    uint8_t tx_key[AES_GCM_KEY_LEN];
    uint8_t rx_key[AES_GCM_KEY_LEN];
} pqc_conn_t;

// Connection lifecycle API
pqc_conn_t* pqc_listen_and_accept(int port);
pqc_conn_t* pqc_connect(const char *host_or_ip, int port);
void        pqc_close(pqc_conn_t *conn);

// Post-quantum application data transfer (AES-256-GCM AEAD over TLS channel)
int pqc_send(pqc_conn_t *conn, const void *buf, size_t total_bytes);
int pqc_recv(pqc_conn_t *conn, void *buf, size_t max_bytes);

// Wire framing helpers
int tls_send_exact(SSL *ssl, const void *buf, size_t total_bytes);
int tls_recv_exact(SSL *ssl, void *buf, size_t total_bytes);

#endif /* HANDSHAKE_H */
