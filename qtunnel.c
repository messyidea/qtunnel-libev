#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <sodium.h>
#include <openssl/rc4.h>
#include "qtunnel.h"

struct struct_options options;
//struct tunnel qtunnel;
char *short_opts = "b:c:l:g:s:";
static struct option long_opts[] = {
    {"backend", required_argument, NULL, 'b'},
    {"clientmode", required_argument, NULL, 'c'},
    {"listen", required_argument, NULL, 'l'},
    {"logto", required_argument, NULL, 'g'},
    {"secret", required_argument, NULL, 's'},
    {0, 0, 0, 0}
};

int serv_sock, clnt_sock, remote_sock;
struct sockaddr_in serv_adr, clnt_adr, remote_adr;
int clnt_adr_size;

int main(int argc, char *argv[]) {
//    while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
//        switch(c) {
//            case 'b':
//
//        }
//    }
    //get_parm();
    if (sodium_init() == -1) {
        puts("hehe");
        return 1;
    } else {
        puts("xixi");
    }

    options.faddr = "0.0.0.0:8765";
    options.baddr = "127.0.0.1:8766";
    options.cryptoMethod = "RC4";
    options.secret = "secret";
    options.clientMod = 1;
    puts("ok");

    char key[] = "secret";
    char origin[] = "123123123123";
    char tmp[256], tmp2[256];
    printf("sizedof = %d\n",sizeof(tmp));
    memset(tmp, 0, sizeof(tmp));
    memset(tmp2, 0, sizeof(tmp2));
    RC4_KEY rc4key;
    RC4_set_key(&rc4key, strlen(key), (const unsigned char*)key);
    RC4(&rc4key, strlen(origin), (const unsigned char*)origin, tmp);
    printf("tmp ==  %s\n", tmp);
    RC4_set_key(&rc4key, strlen(key), (const unsigned char*)key);
    RC4(&rc4key, strlen(tmp), (const unsigned char*)tmp, tmp2);
    printf("tmp2 == %s\n",tmp2);
    printf("orig == %s\n", origin);

//    build_server();
//
//    while(1) {
//        clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_adr, &clnt_adr_size);
//        handle_client(clnt_sock);
//
//    }

//    while(1) {
//        if(waite_for_client() == 0) {
//            handle_client();
//        }
//    }

    puts("ok 2");
    return 0;




}

int build_server() {
    memset(&serv_adr, 0, sizeof(serv_adr));

    serv_adr.sin_port = htons(atoi("1234"));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);

    serv_sock = socket(AF_INET, SOCK_STREAM, 0);

    bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr));

    listen(serv_sock,1);
}


void handle_client(int clnt_sock) {
    memset(&remote_adr, 0, sizeof(remote_adr));
    remote_adr.sin_family = AF_INET;
    remote_adr.sin_port = htons(atoi("2334"));
    remote_adr.sin_addr.s_addr = inet_addr("127.0.0.1");

    remote_sock = socket(AF_INET, SOCK_STREAM, 0);

    connect(remote_sock, (struct sockaddr *) &remote_adr, sizeof(remote_adr));

    fd_set io;
    char buffer[4096];
    for( ; ; ) {
        FD_ZERO(&io);
        FD_SET(clnt_sock, &io);
        FD_SET(remote_sock, &io);
        select(fd(), &io, NULL, NULL, NULL);
        if(FD_ISSET(clnt_sock, &io)) {
            int count = recv(clnt_sock, buffer, sizeof(buffer), 0);
            send(remote_sock, buffer, count, 0);
        }

        if(FD_ISSET(remote_sock, &io)) {
            int count = recv(remote_sock, buffer, sizeof(buffer), 0);
            send(clnt_sock, buffer, count, 0);
        }
    }

}

int fd() {
    unsigned int fd = clnt_sock;
    if(remote_sock > fd) fd = remote_sock;
    return fd + 1;
}
