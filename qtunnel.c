#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <openssl/rc4.h>
#include <openssl/md5.h>
#include <openssl/evp.h>
#include <sys/ioctl.h>
#include "qtunnel.h"

struct struct_options options;
struct struct_setting setting;

int serv_sock;
struct sockaddr_in serv_adr;
//int clnt_adr_size;
int *cofds;
int *lofds;
int *isused;
int colen;
//struct sockaddr_in *clnt_adr;
//struct sockaddr_in *remote_adr;
RC4_KEY *cokey;
RC4_KEY *lokey;
int maxfd;
byte** coinput;
byte** cooutput;
const int BUFSIZE = 40960;
#define SAFE_REALLOC(x, y, z) safeRealloc((void **) (x), (y), (z))
int safeRealloc(void **data, int oldsize, int newsize);

int main(int argc, char *argv[]){
    int i;
    colen = 10;
    maxfd = -1;
    cofds = (int *)malloc(sizeof(int) * colen);
    lofds = (int *)malloc(sizeof(int) * colen);
    isused = (int *)malloc(sizeof(int) * colen);
    //clnt_adr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in) * colen);
    //remote_adr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in) * colen);
    coinput = (byte **)malloc(sizeof(byte*) * colen);
    cooutput = (byte **)malloc(sizeof(byte*) * colen);
    cokey = (RC4_KEY *)malloc(sizeof(RC4_KEY) * colen);
    lokey = (RC4_KEY *)malloc(sizeof(RC4_KEY) * colen);

    //clear
    for(i = 0; i < colen; ++i) {
        isused[i] = 0;
        coinput[i] = (byte*)malloc(sizeof(byte) * BUFSIZE);
        cooutput[i] = (byte*)malloc(sizeof(byte) * BUFSIZE);
    }


    //malloc buf



    get_param(argc, argv);

//    options.faddr = "0.0.0.0:8765";
//    options.baddr = "";
//    options.cryptoMethod = "RC4";
//    options.secret = "secret";
//    options.clientMod = 1;

    build_server();

    while(1) {
        fd_set io;
        FD_ZERO(&io);
        FD_SET(serv_sock, &io);
        for(i = 0; i < colen; ++i) {
            if(isused[i] != 0) {
                FD_SET(cofds[i], &io);
                FD_SET(lofds[i], &io);
            }
        }
        if ( select(maxfd+1, &io, NULL, NULL, NULL) < 0) {
            perror("select error!");
            exit(1);
        }
        if(FD_ISSET(serv_sock, &io)) {
            handle_accept();
        }

        for(i = 0; i < colen; ++i) {
            if(isused[i] != 0 && FD_ISSET(cofds[i], &io)) {
                handle_local(i);
            }
            if(isused[i] != 0 && FD_ISSET(lofds[i], &io)) {
                handle_remote(i);
            }
        }

    }

    return 0;
}

void handle_local(int pos) {
    //printf("size == %d\n", sizeof(coinput[pos]));
    int count = recv(cofds[pos], coinput[pos], BUFSIZE, 0);
    if(count < 0) {
        perror("error count");
        close(cofds[pos]);
        close(lofds[pos]);
        isused[pos] = 0;
        return ;
    }
    if(count == 0) {
        close(cofds[pos]);
        close(lofds[pos]);
        isused[pos] = 0;
        return ;
    }
    printf("log : read %d byte from client\n", count);

    //memset(buffer2, 0, sizeof(buffer2));
    RC4(&cokey[pos], count, coinput[pos], cooutput[pos]);

    send(lofds[pos], cooutput[pos], count, 0);
    puts("send ok");
}

void handle_remote(int pos) {
    //printf("size == %d\n", sizeof(coinput[pos]));
    int count = recv(lofds[pos], coinput[pos], BUFSIZE, 0);
    if(count < 0) {
        perror("error count");
        close(cofds[pos]);
        close(lofds[pos]);
        isused[pos] = 0;
        return ;
    }
    if(count == 0) {
        close(cofds[pos]);
        close(lofds[pos]);
        isused[pos] = 0;
        return ;
    }
    printf("log : read %d byte from remote\n", count);

    //memset(buffer2, 0, sizeof(buffer2));
    RC4(&lokey[pos], count, coinput[pos], cooutput[pos]);

    send(cofds[pos], cooutput[pos], count, 0);
    puts("send ok");
}

void handle_accept() {
    int nfd, i, remote_sock, j, o;
    int clnt_adr_size;
    struct sockaddr_in addr, remote_adr;
    nfd = accept(serv_sock, (struct sockaddr*) &addr, &clnt_adr_size);

    //printf("nfd == %d\n", nfd);

    if(nfd == -1) return ;
    if(nfd > maxfd) maxfd = nfd;
    j = 1;
    ioctl(nfd, FIONBIO, &j);
    j = 0;
    setsockopt(nfd, SOL_SOCKET, SO_LINGER, &j, sizeof(j));
    int pos = -1;

    for(i = 0; i < colen; ++i) {
        if(isused[i] == 0) {
            pos = i;
            break;
        }
    }

    if(pos == -1) {
        //printf("------------------------------------------------------------------large to %d\n", colen * 2);
        o = colen;
        colen = colen * 2;
        if (!SAFE_REALLOC(&cofds, sizeof(int) * o, sizeof(int) * colen)) {
			goto Shortage;
		}
        if (!SAFE_REALLOC(&lofds, sizeof(int) * o, sizeof(int) * colen)) {
			goto Shortage;
		}
        if (!SAFE_REALLOC(&cokey, sizeof(RC4_KEY) * o, sizeof(RC4_KEY) * colen)) {
			goto Shortage;
		}
        if (!SAFE_REALLOC(&lokey, sizeof(RC4_KEY) * o, sizeof(RC4_KEY) * colen)) {
			goto Shortage;
		}
        if (!SAFE_REALLOC(&coinput, sizeof(byte *) * o, sizeof(byte *) * colen)) {
			goto Shortage;
		}
        if (!SAFE_REALLOC(&cooutput, sizeof(byte *) * o, sizeof(byte *) * colen)) {
			goto Shortage;
		}
        for(i = o; i < colen ; ++i) {
            isused[i] = 0;
            coinput[i] = (byte *)malloc(sizeof(byte) * BUFSIZE);
            cooutput[i] = (byte *)malloc(sizeof(byte) * BUFSIZE);
        }
        pos = o;
    }


    if(pos != -1) {
        //printf("pos ====================================================================================== %d\n", pos);
        isused[pos] = 1;
        cofds[pos] = nfd;
        memset(&remote_adr, 0, sizeof(remote_adr));
        remote_adr.sin_family = AF_INET;
        remote_adr.sin_port = htons(atoi(setting.baddr_port));
        remote_adr.sin_addr.s_addr = inet_addr(setting.baddr_host);

        remote_sock = socket(PF_INET, SOCK_STREAM, 0);
        //printf("socks == %d   |   %d\n", nfd, remote_sock);

        if(remote_sock < 0) {
            perror("socket error");
            isused[pos] = 0;
            return ;
        }

        j = 0;
        setsockopt(remote_sock, SOL_SOCKET, SO_LINGER, &j, sizeof(j));


        lofds[pos] = remote_sock;

        if(remote_sock > maxfd) maxfd = remote_sock;


        if ( connect(remote_sock, (struct sockaddr *) &remote_adr, sizeof(remote_adr)) < 0) {
            perror("connect remote error");
            exit(1);
        }

        j = 1;
	    ioctl(remote_sock, FIONBIO, &j);

        RC4_set_key(&cokey[pos], 16, setting.secret);
        RC4_set_key(&lokey[pos], 16, setting.secret);
        return ;
    }
Shortage:
    perror("out of memory!");
    exit(1);
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
    byte buf[16];
    byte buf2[16];
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

    if(serv_sock > maxfd) {
        maxfd = serv_sock;
    }

    if(serv_sock < 0) {
        perror("socket error");
        exit(1);
    }

    int optval = 1;
    if(setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt error");
        exit(1);
    }
    int j = 1;
    ioctl(serv_sock, FIONBIO, &j);
    if ( bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
        perror("bind error");
        exit(1);
    }

    if( listen(serv_sock,5) == -1 ) {
        perror("listen error");
        exit(1);
    }
}

int safeRealloc(void **data, int oldsize, int newsize)
{
	void *newData = malloc(newsize + 1);
	if (!newData) {
		return 0;
	}
	if (newsize < oldsize) {
		memcpy(newData, *data, newsize);
	} else {
		memcpy(newData, *data, oldsize);
	}
	*data = newData;
	return 1;
}
