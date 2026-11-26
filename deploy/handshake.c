#include "handshake.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

/* Helper to find certificate files */
static const char* find_file(const char *rel_path, const char *alt_path, const char *abs_path)
{
    if (access(rel_path, R_OK) == 0) return rel_path;
    if (access(alt_path, R_OK) == 0) return alt_path;
    if (access(abs_path, R_OK) == 0) return abs_path;
    return rel_path;
}

int tls_send_exact(SSL *ssl, const void *buf, size_t total_bytes)
{
    size_t sent = 0;
    while (sent < total_bytes) {
        int n = SSL_write(ssl, (const char *)buf + sent, (int)(total_bytes - sent));
        if (n <= 0) {
            int err = SSL_get_error(ssl, n);
            fprintf(stderr, "[!] SSL_write failed with code %d (error %d)\n", n, err);
            return -1;
        }
        sent += (size_t)n;
    }
    return 0;
}

int tls_recv_exact(SSL *ssl, void *buf, size_t total_bytes)
{
    size_t received = 0;
    while (received < total_bytes) {
        int n = SSL_read(ssl, (char *)buf + received, (int)(total_bytes - received));
        if (n <= 0) {
            int err = SSL_get_error(ssl, n);
            fprintf(stderr, "[!] SSL_read failed with code %d (error %d)\n", n, err);
            return -1;
        }
        received += (size_t)n;
    }
    return 0;
}

/* Derive directional 256-bit symmetric keys from shared secret */
static void derive_keys(const uint8_t *shared_secret, uint8_t *c2s_key, uint8_t *s2c_key)
{
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    const char *label_c2s = "pqc-c2s";
    const char *label_s2c = "pqc-s2c";

    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, shared_secret, CRYPTO_BYTES);
    EVP_DigestUpdate(ctx, label_c2s, strlen(label_c2s));
    EVP_DigestFinal_ex(ctx, c2s_key, NULL);

    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, shared_secret, CRYPTO_BYTES);
    EVP_DigestUpdate(ctx, label_s2c, strlen(label_s2c));
    EVP_DigestFinal_ex(ctx, s2c_key, NULL);

    EVP_MD_CTX_free(ctx);
}

pqc_conn_t* pqc_listen_and_accept(int port)
{
    int listen_fd = -1;
    int client_fd = -1;
    struct sockaddr_in saddr, caddr;
    socklen_t clen = sizeof(caddr);
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    pqc_conn_t *conn = NULL;

    const char *crt_path = find_file("certs/server.crt", "../certs/server.crt", "/home/vinayak/Vinayak/projects/PQC/PQC_OELP/certs/server.crt");
    const char *key_path = find_file("certs/server.key", "../certs/server.key", "/home/vinayak/Vinayak/projects/PQC/PQC_OELP/certs/server.key");

    ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        fprintf(stderr, "[!] Failed to create server SSL_CTX\n");
        return NULL;
    }
    SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);

    if (SSL_CTX_use_certificate_file(ctx, crt_path, SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM) <= 0) {
        fprintf(stderr, "[!] Failed to load certificate (%s) or private key (%s)\n", crt_path, key_path);
        SSL_CTX_free(ctx);
        return NULL;
    }

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("[!] socket");
        SSL_CTX_free(ctx);
        return NULL;
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons((uint16_t)port);
    saddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        perror("[!] bind");
        close(listen_fd);
        SSL_CTX_free(ctx);
        return NULL;
    }

    if (listen(listen_fd, 5) < 0) {
        perror("[!] listen");
        close(listen_fd);
        SSL_CTX_free(ctx);
        return NULL;
    }

    printf("[+] Server listening on port %d for TLS 1.3 + NewHope-1024 connections...\n", port);
    client_fd = accept(listen_fd, (struct sockaddr *)&caddr, &clen);
    close(listen_fd);
    if (client_fd < 0) {
        perror("[!] accept");
        SSL_CTX_free(ctx);
        return NULL;
    }

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client_fd);
    if (SSL_accept(ssl) <= 0) {
        fprintf(stderr, "[!] TLS handshake failed\n");
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(client_fd);
        SSL_CTX_free(ctx);
        return NULL;
    }
    printf("[+] Classical TLS 1.3 handshake established (%s, %s)\n",
           SSL_get_version(ssl), SSL_get_cipher(ssl));

    /* Post-Quantum NewHope-1024 Key Encapsulation Exchange */
    uint8_t pk[CRYPTO_PUBLICKEYBYTES];
    uint8_t sk[CRYPTO_SECRETKEYBYTES];
    if (newhope_keygen(pk, sk) != 0) {
        fprintf(stderr, "[!] Server NewHope keypair generation failed\n");
        SSL_free(ssl);
        close(client_fd);
        SSL_CTX_free(ctx);
        return NULL;
    }

    /* Step 1: Server sends Public Key frame */
    frame_header_t hdr;
    hdr.type = MSG_TYPE_PUBLIC_KEY;
    hdr.length = htonl(CRYPTO_PUBLICKEYBYTES);
    if (tls_send_exact(ssl, &hdr, sizeof(hdr)) < 0 ||
        tls_send_exact(ssl, pk, CRYPTO_PUBLICKEYBYTES) < 0) {
        fprintf(stderr, "[!] Server failed to send NewHope public key\n");
        SSL_free(ssl);
        close(client_fd);
        SSL_CTX_free(ctx);
        return NULL;
    }

    /* Step 2: Server receives Ciphertext frame from Client */
    if (tls_recv_exact(ssl, &hdr, sizeof(hdr)) < 0) {
        fprintf(stderr, "[!] Failed to receive ciphertext header\n");
        SSL_free(ssl);
        close(client_fd);
        SSL_CTX_free(ctx);
        return NULL;
    }
    uint32_t ct_len = ntohl(hdr.length);
    if (hdr.type != MSG_TYPE_CIPHERTEXT || ct_len != CRYPTO_CIPHERTEXTBYTES) {
        fprintf(stderr, "[!] Invalid ciphertext frame (type: %d, len: %u)\n", hdr.type, ct_len);
        SSL_free(ssl);
        close(client_fd);
        SSL_CTX_free(ctx);
        return NULL;
    }

    uint8_t ct[CRYPTO_CIPHERTEXTBYTES];
    if (tls_recv_exact(ssl, ct, CRYPTO_CIPHERTEXTBYTES) < 0) {
        fprintf(stderr, "[!] Failed to receive NewHope ciphertext\n");
        SSL_free(ssl);
        close(client_fd);
        SSL_CTX_free(ctx);
        return NULL;
    }

    /* Step 3: Server decapsulates to recover shared secret */
    conn = (pqc_conn_t *)calloc(1, sizeof(pqc_conn_t));
    if (!conn) {
        SSL_free(ssl);
        close(client_fd);
        SSL_CTX_free(ctx);
        return NULL;
    }
    conn->socket_fd = client_fd;
    conn->ssl = ssl;
    conn->ctx = ctx;

    if (newhope_shareda(conn->shared_secret, sk, ct) != 0) {
        fprintf(stderr, "[!] Server decapsulation failed\n");
        pqc_close(conn);
        return NULL;
    }

    /* Step 4: Derive directional keys */
    uint8_t c2s_key[AES_GCM_KEY_LEN];
    uint8_t s2c_key[AES_GCM_KEY_LEN];
    derive_keys(conn->shared_secret, c2s_key, s2c_key);
    memcpy(conn->tx_key, s2c_key, AES_GCM_KEY_LEN);
    memcpy(conn->rx_key, c2s_key, AES_GCM_KEY_LEN);

    /* Step 5: Send Success frame */
    hdr.type = MSG_TYPE_SUCCESS;
    hdr.length = 0;
    tls_send_exact(ssl, &hdr, sizeof(hdr));

    printf("[+] NewHope-1024 Post-Quantum key exchange successfully finalized!\n");
    return conn;
}

pqc_conn_t* pqc_connect(const char *host_or_ip, int port)
{
    int sock_fd = -1;
    struct sockaddr_in saddr;
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    pqc_conn_t *conn = NULL;

    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        fprintf(stderr, "[!] Failed to create client SSL_CTX\n");
        return NULL;
    }
    SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL); /* For self-signed test cert */

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("[!] socket");
        SSL_CTX_free(ctx);
        return NULL;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, host_or_ip, &saddr.sin_addr) <= 0) {
        perror("[!] inet_pton");
        close(sock_fd);
        SSL_CTX_free(ctx);
        return NULL;
    }

    if (connect(sock_fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        perror("[!] connect");
        close(sock_fd);
        SSL_CTX_free(ctx);
        return NULL;
    }

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock_fd);
    if (SSL_connect(ssl) <= 0) {
        fprintf(stderr, "[!] TLS handshake failed\n");
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sock_fd);
        SSL_CTX_free(ctx);
        return NULL;
    }
    printf("[+] Classical TLS 1.3 handshake established (%s, %s)\n",
           SSL_get_version(ssl), SSL_get_cipher(ssl));

    /* Step 1: Client receives Server Public Key */
    frame_header_t hdr;
    if (tls_recv_exact(ssl, &hdr, sizeof(hdr)) < 0) {
        fprintf(stderr, "[!] Failed to receive public key header\n");
        SSL_free(ssl);
        close(sock_fd);
        SSL_CTX_free(ctx);
        return NULL;
    }
    uint32_t pk_len = ntohl(hdr.length);
    if (hdr.type != MSG_TYPE_PUBLIC_KEY || pk_len != CRYPTO_PUBLICKEYBYTES) {
        fprintf(stderr, "[!] Invalid public key frame (type: %d, len: %u)\n", hdr.type, pk_len);
        SSL_free(ssl);
        close(sock_fd);
        SSL_CTX_free(ctx);
        return NULL;
    }

    uint8_t pk[CRYPTO_PUBLICKEYBYTES];
    if (tls_recv_exact(ssl, pk, CRYPTO_PUBLICKEYBYTES) < 0) {
        fprintf(stderr, "[!] Failed to receive public key bytes\n");
        SSL_free(ssl);
        close(sock_fd);
        SSL_CTX_free(ctx);
        return NULL;
    }

    /* Step 2: Client encapsulates shared secret */
    conn = (pqc_conn_t *)calloc(1, sizeof(pqc_conn_t));
    if (!conn) {
        SSL_free(ssl);
        close(sock_fd);
        SSL_CTX_free(ctx);
        return NULL;
    }
    conn->socket_fd = sock_fd;
    conn->ssl = ssl;
    conn->ctx = ctx;

    uint8_t ct[CRYPTO_CIPHERTEXTBYTES];
    if (newhope_sharedb(conn->shared_secret, ct, pk) != 0) {
        fprintf(stderr, "[!] Client encapsulation failed\n");
        pqc_close(conn);
        return NULL;
    }

    /* Step 3: Client sends Ciphertext frame to Server */
    hdr.type = MSG_TYPE_CIPHERTEXT;
    hdr.length = htonl(CRYPTO_CIPHERTEXTBYTES);
    if (tls_send_exact(ssl, &hdr, sizeof(hdr)) < 0 ||
        tls_send_exact(ssl, ct, CRYPTO_CIPHERTEXTBYTES) < 0) {
        fprintf(stderr, "[!] Failed to send ciphertext\n");
        pqc_close(conn);
        return NULL;
    }

    /* Step 4: Client receives Success confirmation frame */
    if (tls_recv_exact(ssl, &hdr, sizeof(hdr)) < 0 || hdr.type != MSG_TYPE_SUCCESS) {
        fprintf(stderr, "[!] Server did not confirm handshake\n");
        pqc_close(conn);
        return NULL;
    }

    /* Step 5: Derive directional keys */
    uint8_t c2s_key[AES_GCM_KEY_LEN];
    uint8_t s2c_key[AES_GCM_KEY_LEN];
    derive_keys(conn->shared_secret, c2s_key, s2c_key);
    memcpy(conn->tx_key, c2s_key, AES_GCM_KEY_LEN);
    memcpy(conn->rx_key, s2c_key, AES_GCM_KEY_LEN);

    printf("[+] NewHope-1024 Post-Quantum key exchange successfully finalized!\n");
    return conn;
}

void pqc_close(pqc_conn_t *conn)
{
    if (!conn) return;
    if (conn->ssl) {
        SSL_shutdown(conn->ssl);
        SSL_free(conn->ssl);
    }
    if (conn->socket_fd >= 0) close(conn->socket_fd);
    if (conn->ctx) SSL_CTX_free(conn->ctx);
    memset(conn, 0, sizeof(*conn));
    free(conn);
}

int pqc_send(pqc_conn_t *conn, const void *buf, size_t total_bytes)
{
    if (!conn || !conn->ssl || !buf) return -1;

    uint8_t iv[AES_GCM_IV_LEN];
    if (RAND_bytes(iv, AES_GCM_IV_LEN) <= 0) return -1;

    uint8_t *ciphertext = (uint8_t *)malloc(total_bytes);
    if (!ciphertext) return -1;

    uint8_t tag[AES_GCM_TAG_LEN];
    int len = 0;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) { free(ciphertext); return -1; }

    EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, AES_GCM_IV_LEN, NULL);
    EVP_EncryptInit_ex(ctx, NULL, NULL, conn->tx_key, iv);
    EVP_EncryptUpdate(ctx, ciphertext, &len, (const unsigned char *)buf, (int)total_bytes);
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, AES_GCM_TAG_LEN, tag);
    EVP_CIPHER_CTX_free(ctx);

    /* Framing: Header || IV || Ciphertext || Tag */
    uint32_t payload_len = AES_GCM_IV_LEN + (uint32_t)total_bytes + AES_GCM_TAG_LEN;
    frame_header_t hdr;
    hdr.type = MSG_TYPE_APP_DATA;
    hdr.length = htonl(payload_len);

    if (tls_send_exact(conn->ssl, &hdr, sizeof(hdr)) < 0 ||
        tls_send_exact(conn->ssl, iv, AES_GCM_IV_LEN) < 0 ||
        tls_send_exact(conn->ssl, ciphertext, total_bytes) < 0 ||
        tls_send_exact(conn->ssl, tag, AES_GCM_TAG_LEN) < 0) {
        free(ciphertext);
        return -1;
    }

    free(ciphertext);
    return (int)total_bytes;
}

int pqc_recv(pqc_conn_t *conn, void *buf, size_t max_bytes)
{
    if (!conn || !conn->ssl || !buf) return -1;

    frame_header_t hdr;
    if (tls_recv_exact(conn->ssl, &hdr, sizeof(hdr)) < 0) return -1;

    if (hdr.type != MSG_TYPE_APP_DATA) {
        fprintf(stderr, "[!] Expected APP_DATA frame, got %d\n", hdr.type);
        return -1;
    }

    uint32_t payload_len = ntohl(hdr.length);
    if (payload_len < AES_GCM_IV_LEN + AES_GCM_TAG_LEN) return -1;

    uint32_t ct_len = payload_len - AES_GCM_IV_LEN - AES_GCM_TAG_LEN;
    if (ct_len > max_bytes) {
        fprintf(stderr, "[!] Buffer overflow: payload %u exceeds max %zu\n", ct_len, max_bytes);
        return -1;
    }

    uint8_t iv[AES_GCM_IV_LEN];
    uint8_t tag[AES_GCM_TAG_LEN];
    uint8_t *ciphertext = (uint8_t *)malloc(ct_len);
    if (!ciphertext) return -1;

    if (tls_recv_exact(conn->ssl, iv, AES_GCM_IV_LEN) < 0 ||
        tls_recv_exact(conn->ssl, ciphertext, ct_len) < 0 ||
        tls_recv_exact(conn->ssl, tag, AES_GCM_TAG_LEN) < 0) {
        free(ciphertext);
        return -1;
    }

    int len = 0;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) { free(ciphertext); return -1; }

    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, AES_GCM_IV_LEN, NULL);
    EVP_DecryptInit_ex(ctx, NULL, NULL, conn->rx_key, iv);
    EVP_DecryptUpdate(ctx, (unsigned char *)buf, &len, ciphertext, (int)ct_len);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, AES_GCM_TAG_LEN, tag);

    int ret = EVP_DecryptFinal_ex(ctx, (unsigned char *)buf + len, &len);
    EVP_CIPHER_CTX_free(ctx);
    free(ciphertext);

    if (ret <= 0) {
        fprintf(stderr, "[!] AES-256-GCM authentication tag check failed!\n");
        return -1;
    }

    return (int)ct_len;
}
