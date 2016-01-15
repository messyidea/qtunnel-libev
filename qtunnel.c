#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include "qtunnel.h"

struct struct_options options;
struct tunnel qtunnel;
char *short_opts = "b:c:l:g:s:";
static struct option long_opts[] = {
    {"backend", required_argument, NULL, 'b'},
    {"clientmode", required_argument, NULL, 'c'},
    {"listen", required_argument, NULL, 'l'},
    {"logto", required_argument, NULL, 'g'},
    {"secret", required_argument, NULL, 's'},
    {0, 0, 0, 0}
};

int main(int argc, char *argv[]) {
//    while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
//        switch(c) {
//            case 'b':
//
//        }
//    }
    options.faddr = "0.0.0.0:8765";
    options.baddr = "127.0.0.1:8766";
    options.cryptoMethod = "RC4";
    options.secret = "secret";
    options.clientMod = 1;
    puts("ok");

    build_server();

    while(1) {
        if(waite_for_client() == 0) {
            handle_client();
        }
    }

    return 0;
    puts("ok 2");



}

int build_server() {
    memset(&qtunnel.server_addr, 0, sizeof(qtunnel.server_addr));

    qtunnel.server_addr.sin_port = htons(atoi("1234"));
    qtunnel.server_addr.sin_family = AF_INET;
    qtunnel.server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    qtunnel.server_socket = socket(AF_INET, SOCK_STREAM, 0);

    bind(qtunnel.server_socket, (struct sockaddr*)&qtunnel.server_addr, sizeof(qtunnel.server_addr));

    listen(qtunnel.server_socket,1);
}
