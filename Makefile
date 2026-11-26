CC ?= gcc
CFLAGS ?= -O3 -Wall -Wextra -Wno-unused-function -Iinclude -DNEWHOPE_N=1024
LDFLAGS ?= -lssl -lcrypto

SRC = src/fips202.c src/precomp.c src/randombytes.c src/reduce.c src/ntt.c src/cbd.c src/poly.c src/cpapke.c src/verify.c src/newhope.c
OBJ = $(SRC:.c=.o)

BINARIES = server client test_newhope test_ntt test_cbd test_kat benchmark

all: $(BINARIES)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

server: deploy/server.c deploy/handshake.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

client: deploy/client.c deploy/handshake.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_newhope: tests/test_newhope.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_ntt: tests/test_ntt.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_cbd: tests/test_cbd.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_kat: tests/test_kat.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

benchmark: tests/benchmark.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test: test_newhope test_ntt test_cbd test_kat
	@echo "=== Running test_ntt ==="
	./test_ntt
	@echo "\n=== Running test_cbd ==="
	./test_cbd
	@echo "\n=== Running test_kat ==="
	./test_kat
	@echo "\n=== Running test_newhope ==="
	./test_newhope

bench: benchmark
	./benchmark

run: server client
	@echo "=== Starting Post-Quantum TLS 1.3 Server ==="
	@./server 8443 & SERVER_PID=$$!; \
	sleep 1; \
	echo "=== Connecting Post-Quantum TLS 1.3 Client ==="; \
	./client 127.0.0.1 8443; \
	wait $$SERVER_PID 2>/dev/null || true

clean:
	rm -f $(OBJ) $(BINARIES)

.PHONY: all test bench run clean
