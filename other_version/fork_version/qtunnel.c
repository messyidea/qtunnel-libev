#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>
#include <openssl/rc4.h>
#include <openssl/md5.h>
#include <openssl/evp.h>
#include "qtunnel.h"

struct struct_options options;
struct struct_setting setting;

int serv_sock, clnt_sock, remote_sock;
struct sockaddr_in serv_adr, clnt_adr, remote_adr;
int clnt_adr_size;

void sigCatcher(int n) {
    printf("a child process dies\n");
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[]){

    signal(SIGCHLD, sigCatcher);

    get_param(argc, argv);

//    options.faddr = "0.0.0.0:8765";
//    options.baddr = "";
//    options.cryptoMethod = "RC4";
//    options.secret = "secret";
//    options.clientMod = 1;

    build_server();

    while(1) {
        clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_adr, &clnt_adr_size);
        if(fork() == 0) {
            close(serv_sock);
            handle_client(clnt_sock);
            exit(0);
        }
        close(clnt_sock);
    }

    return 0;
}

void get_param(int argc, char *argv[]) {
    char c;
    unsigned long p;
    while((c = getopt_long (argc, argv, short_opts, long_opts, NULL)) != -1) {
        switch(c) {
            case 'h': {
                print_usage();
                exit(0);
            }
            case 'b': {
                options.baddr = optarg;
                p = strchr(optarg, ':') - optarg;
                strncpy(setting.baddr_host, optarg, p);
                strcpy(setting.baddr_port, optarg + p + 1);
                //printf("badd = %s  %s\n", setting.baddr_port, setting.baddr_host);
                break;
            }
            case 'l': {
                options.faddr = optarg;
                p = strchr(optarg, ':') - optarg;
                strcpy(setting.faddr_port, optarg + p + 1);
                //printf("fadd = %s\n", setting.faddr_port);
                break;
            }
            case 'c': {
                options.clientMod = optarg;
                if(strcmp(optarg, "true") == 0) {
                    setting.clientMod = CLIENTMOD;
                } else {
                    setting.clientMod = SERVERMOD;
                }
                break;
            }
            case 's': {
                options.secret = optarg;
                strncpy(setting.secret, secretToKey(optarg, 16), 16);
                //setting.secret = secretToKey(optarg, 16);
                //printf("sec == %s\n", setting.secret);
                break;
            }
            default: {
                printf("unknow option of %c\n", optopt);
                break;
            }
        }
    }
    if(strcmp(setting.baddr_port, "") == 0) {
        perror("missing option --backend");
        exit(1);
    }
    if(strcmp(setting.baddr_host, "") == 0) {
        perror("missing option --backend");
        exit(1);
    }
    if(strcmp(setting.secret, "") == 0) {
        perror("mission option --secret");
        exit(1);
    }

    //printf("%s %s %s\n",setting.faddr_port, setting.baddr_port, setting.baddr_host);
}

void print_usage() {
    printf("Options:\n\
  --help\n\
  --backend=remotehost:remoteport    remote\n\
  --listen=localhost:localport   local\n\
  --clientmod=true or false  buffer size\n\
  --secret=secret secret\
\n");
}

byte* secretToKey(char* sec, int size) {
    byte *buf = malloc(sizeof(byte) * 16);
    byte *buf2 = malloc(sizeof(byte) * 16);
    MD5_CTX h;
    MD5_Init(&h);
    int count = size / 16;
    int i,j;
    for(i = 0; i < count; ++i) {
        MD5_Update(&h, sec, strlen(sec));
        MD5_Final(buf2, &h);
        strncpy(buf, buf2, 15);
    }
    buf[15]=0;

    return buf;
}

int build_server() {
    memset(&serv_adr, 0, sizeof(serv_adr));

    serv_adr.sin_port = htons(atoi(setting.faddr_port));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    if(serv_sock < 0) {
        perror("socket error");
        exit(1);
    }

    int optval = 1;
    if(setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt error");
        exit(1);
    }

    if ( bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
        perror("bind error");
        exit(1);
    }

    if( listen(serv_sock,1) == -1 ) {
        perror("listen error");
        exit(1);
    }
}


void handle_client(int clnt_sock) {
    memset(&remote_adr, 0, sizeof(remote_adr));

    //printf("bass == %s %s \n", setting.baddr_host, setting.baddr_port);
    remote_adr.sin_family = AF_INET;
    remote_adr.sin_port = htons(atoi(setting.baddr_port));
    remote_adr.sin_addr.s_addr = inet_addr(setting.baddr_host);

    remote_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(remote_sock < 0) {
        perror("build remote error");
        exit(1);
    }

    if ( connect(remote_sock, (struct sockaddr *) &remote_adr, sizeof(remote_adr)) < 0) {
        perror("connect remote error");
        exit(1);
    }

    fd_set io;
    byte buffer[40960];
    byte buffer2[40960];
    RC4_KEY rc4key;
    RC4_KEY rc4key2;

    RC4_set_key(&rc4key, 16, setting.secret);
    RC4_set_key(&rc4key2, 16, setting.secret);
    //printf("secret = %s\n",setting.secret);
    for( ; ; ) {
        FD_ZERO(&io);
        FD_SET(clnt_sock, &io);
        FD_SET(remote_sock, &io);
        if( select(maxfd(), &io, NULL, NULL, NULL) < 0){
            puts("select error");
            break;
        }
        if(FD_ISSET(clnt_sock, &io)) {
            int count = recv(clnt_sock, buffer, sizeof(buffer), 0);
            if(count < 0) {
                perror("error count");
                close(clnt_sock);
                close(remote_sock);
                return ;
            }
            if(count == 0) {
                printf("count 0");
                close(clnt_sock);
                close(remote_sock);
                return ;
            }
            printf("log : read %d byte from client\n", count);

            //memset(buffer2, 0, sizeof(buffer2));
            RC4(&rc4key, count, buffer, buffer2);

            send(remote_sock, buffer2, count, 0);
        }

        if(FD_ISSET(remote_sock, &io)) {
            int count = recv(remote_sock, buffer, sizeof(buffer), 0);
            if(count < 0) {
                perror("error count");
                close(clnt_sock);
                close(remote_sock);
                return ;
            }
            if(count == 0) {
                puts("count 0");
                close(clnt_sock);
                close(remote_sock);
                return ;
            }
            printf("log : read %d byte from remote\n", count);
            //memset(buffer2, 0, sizeof(buffer2));
            RC4(&rc4key2, count, buffer, buffer2);

            send(clnt_sock, buffer2, count, 0);
        }
    }

}

unsigned int maxfd() {
    unsigned int fd = clnt_sock;
    if(remote_sock > fd) fd = remote_sock;
    return fd + 1;
}
