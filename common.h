#ifndef COMMON_H
#define COMMON_H

#include <string.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include "reentrant.h"
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <resolv.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <termios.h>
#include <stdbool.h>
#include <malloc.h>

#ifndef WIN32
#include <pthread.h>
#define THREAD_CC
#define THREAD_TYPE						pthread_t
#define THREAD_CREATE(tid, entry, arg)	pthread_create(&(tid), NULL, (entry), (arg))

#else
#include <windows.h>
#define THREAD_CC
#define THREAD_TYPE
#define THREAD_CREATE(tid, entry, arg) do{ _beginthread((entry), 0, (arg)); (tid) = GetCurrentThreadId();}while(0)

#endif

#define int_error(msg) handle_error(__FILE__, __LINE__, msg)
void handle_error(const char *file, int lineno, const char *msg);

void init_OpenSSL(void);

int verify_callback(int ok, X509_STORE_CTX *store);

long post_connectiono_check(SSL *ssl, char *host);

void seed_prng(void);


#endif
