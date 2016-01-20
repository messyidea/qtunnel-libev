#ifndef QTUNNEL_H
#define QTUNNEL_H
#include <getopt.h>
#include <unistd.h>
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

char *short_opts = "hb:c:l:g:s:";

static struct option long_opts[] = {
        {"help", no_argument, NULL, 'h'},
        {"backend", required_argument, NULL, 'b'},
        {"clientmode", required_argument, NULL, 'c'},
        {"listen", required_argument, NULL, 'l'},
        {"logto", required_argument, NULL, 'g'},
        {"secret", required_argument, NULL, 's'},
        {0, 0, 0, 0}
};

byte* secretToKey(char* sec, int size);
void get_param(int argc, char *argv[]);
void print_usage();
int maxfd(int a, int b);
void* handle_client(void *arg);
int build_server();
byte* secretToKey(char* sec, int size);


#define CLIENTMOD 0
#define SERVERMOD 1

#endif
