#ifndef QTUNNEL_H
#define QTUNNEL_H
#include <getopt.h>
#include <unistd.h>
#include <ev.h>
#include <openssl/rc4.h>
#include <openssl/md5.h>
#include <openssl/evp.h>
typedef unsigned char byte;

struct struct_options {
    char *faddr;
    char *baddr;
    char *cryptoMethod;
    byte *secret;
    char *clientMod;
};

struct struct_setting {
    char baddr_host[20];
    char baddr_port[6];
    char faddr_port[6];
    int cryptoMethod;
    byte secret[16];
    int clientMod;
};

char *short_opts = "hdb:c:l:g:s:";

struct conn_ctx {
    ev_io io;
    ev_timer watcher;
    struct conn *conn;
};

struct conn {
    int fd;
    int buf_len;
    int buf_idx;
    char *buf;
    RC4_KEY key;
    struct conn_ctx *recv_ctx;
    struct conn_ctx *send_ctx;
    struct conn *another;
    int type;
    int connected;
};



static struct option long_opts[] = {
        {"help", no_argument, NULL, 'h'},
        {"backend", required_argument, NULL, 'b'},
        {"clientmode", required_argument, NULL, 'c'},
        {"listen", required_argument, NULL, 'l'},
        {"logto", required_argument, NULL, 'g'},
        {"secret", required_argument, NULL, 's'},
        {"debug", no_argument, NULL, 'd'},
        {0, 0, 0, 0}
};



void free_conn(struct conn *conn);
void close_and_free(EV_P_ struct conn *conn);
void send_cb(EV_P_ ev_io  *watcher, int revents);
void recv_cb(EV_P_ ev_io *watcher, int revents);
byte* secretToKey(char* sec, int size);
void get_param(int argc, char *argv[]);
void print_usage();
int build_server();
byte* secretToKey(char* sec, int size);
void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);
void timeout_cb(EV_P_ ev_timer *watcher, int revents);
void remote_timeout_cb(EV_P_ ev_timer *watcher, int revents);

#define CLIENTMOD 0
#define SERVERMOD 1

#endif
