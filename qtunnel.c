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
#include <openssl/md5.h>
#include "qtunnel.h"

struct struct_options options;
typedef unsigned char byte;
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
byte* secretToKey(char* sec, int size);
byte key[16];

int main(int argc, char *argv[]){
    puts("1");
   // secretToKey(16);
    //key = secretToKey("secret", 16);
    strncpy(key, secretToKey("secret", 16), 16);
    //printf("sec = %s\n", key);
    //RC4_set_key(&rc4key, 16, key);
    puts("2");
//    while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
//        switch(c) {
//            case 'b':
//
//        }
//    }
    //get_parm();
//    if (sodium_init() == -1) {
//        puts("hehe");
//        return 1;
//    } else {
//        puts("xixi");
//    }
//
//    options.faddr = "0.0.0.0:8765";
//    options.baddr = "127.0.0.1:8766";
//    options.cryptoMethod = "RC4";
//    options.secret = "secret";
//    options.clientMod = 1;
//    puts("ok");
//
//    byte key2[] = "testsecret";
//    byte origin[] = "thisISaCLEARtext";
//    byte tmp[256], tmp2[256];
//
//
//    printf("key == %s\n", key);
//    printf("sizedof = %d\n",sizeof(tmp));
//    memset(tmp, 0, sizeof(tmp));
//    memset(tmp2, 0, sizeof(tmp2));
//    RC4_KEY rc4key;
//    RC4_set_key(&rc4key, 16, key);
//    RC4(&rc4key, strlen(origin), origin, tmp);
//    printf("tmp ==  %s\n", tmp);
//    RC4_set_key(&rc4key, 16, key);
//    RC4(&rc4key, strlen(tmp), tmp, tmp2);
//    printf("tmp2 == %s\n",tmp2);
//    printf("orig == %s\n", origin);

    build_server();
    puts("build ok!");

    while(1) {
        clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_adr, &clnt_adr_size);
        if(fork() == 0) {
            close(serv_sock);
            handle_client(clnt_sock);
            exit(0);
        }
        //handle_client(clnt_sock);
    }

//    while(1) {
//        if(waite_for_client() == 0) {
//            handle_client();
//        }
//    }

    puts("ok 2");
    return 0;




}

byte* secretToKey(char* sec, int size) {
    byte buf[16];
    byte buf2[16];
    MD5_CTX h;
    MD5_Init(&h);
    int count = size / 16;
    int i,j;
    printf("count == %d\n", count);
    for(i = 0; i < count; ++i) {
        MD5_Update(&h, sec, strlen(sec));
        MD5_Final(buf2, &h);
        //printf("buf2 == %s\n",buf2);
        strncpy(buf, buf2, 16);
    }
    buf2[15]=0;
    //printf("len == %d\n",strlen(buf2));
    for(i=0;i<16; ++i) {
        printf("%d\n",buf2[i]);
    }

    return buf2;
}

int build_server() {
    memset(&serv_adr, 0, sizeof(serv_adr));

    serv_adr.sin_port = htons(atoi("8765"));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    printf("serv sock == %d\n", serv_sock);

    int optval = 1;
    if(setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        puts("set opt error");
        exit(0);
    }

    if ( bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
        puts("bind error");
        exit(0);
    }

    if( listen(serv_sock,1) == -1 ) {
        puts("listen error");
        exit(0);
    }

}


void handle_client(int clnt_sock) {
    memset(&remote_adr, 0, sizeof(remote_adr));
    remote_adr.sin_family = AF_INET;


    remote_adr.sin_port = htons(atoi("8766"));
    remote_adr.sin_addr.s_addr = inet_addr("23.95.113.");

    remote_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(remote_sock < 0) {
        puts("build remote error");
        exit(0);
    }
    printf("remote sock == %d\n", remote_sock);
    if ( connect(remote_sock, (struct sockaddr *) &remote_adr, sizeof(remote_adr)) < 0) {
        puts("connect remote error");
        exit(0);
    }

    fd_set io;
    byte buffer[4096];
    byte buffer2[4096];
    RC4_KEY rc4key;
    RC4_KEY rc4key2;
    char keykey[] = "secret";
    RC4_set_key(&rc4key, 16, key);
    RC4_set_key(&rc4key2, 16, key);
    for( ; ; ) {
        FD_ZERO(&io);
        FD_SET(clnt_sock, &io);
        FD_SET(remote_sock, &io);
        puts("do zero");
        if( select(fd(), &io, NULL, NULL, NULL) < 0){
            puts("select error");
            break;
        }
        if(FD_ISSET(clnt_sock, &io)) {
            int count = recv(clnt_sock, buffer, sizeof(buffer), 0);
            if(count < 0) {
                puts("error count");
                return ;
            }
            printf("r count == %d\n", count);
            if(count == 0) return ;
            memset(buffer2, 0, sizeof(buffer2));
            printf("buffer = %ds\n", buffer);
            //RC4_set_key(&rc4key, 16, key);
            RC4(&rc4key, count, buffer, buffer2);
            send(remote_sock, buffer2, count, 0);
            puts("send ok");
        }

        if(FD_ISSET(remote_sock, &io)) {
            int count = recv(remote_sock, buffer, sizeof(buffer), 0);
            if(count < 0) {
                puts("error count");
                return ;
            }
            printf("r2 count == %d\n", count);
            if(count == 0) return ;
            memset(buffer2, 0, sizeof(buffer2));
            //RC4_set_key(&rc4key, 16, key);
            RC4(&rc4key2, count, buffer, buffer2);
            send(clnt_sock, buffer2, count, 0);
            puts("send ok");
        }
    }

}

int fd() {
    unsigned int fd = clnt_sock;
    if(remote_sock > fd) fd = remote_sock;
    return fd + 1;
}
