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
#include <fcntl.h>
#include "qtunnel.h"
#include <ev.h>


struct struct_options options;
struct struct_setting setting;

int serv_sock;
struct sockaddr_in serv_adr;
//int clnt_adr_size;
int colen;
//struct sockaddr_in *clnt_adr;
//struct sockaddr_in *remote_adr;

const int BUFSIZE = 40960;
#define SAFE_REALLOC(x, y, z) safeRealloc((void **) (x), (y), (z))
int safeRealloc(void **data, int oldsize, int newsize);
struct tunnel_ctx {
    ev_io io;
    int fd;
    RC4_KEY key;
    char* buf;
    int id;
};
int* isused;
struct tunnel_ctx* lotunnel;
struct tunnel_ctx* cotunnel;


int main(int argc, char *argv[]){
    struct ev_loop *loop = ev_default_loop(0);
    int i;
    colen = 100;
    isused = (int*)malloc(sizeof(int) * colen);
    lotunnel = (struct tunnel_ctx*)malloc(sizeof(struct tunnel_ctx) * colen);
    cotunnel = (struct tunnel_ctx*)malloc(sizeof(struct tunnel_ctx) * colen);

    //clear and malloc buf
    for(i = 0; i < colen; ++i) {
        isused[i] = 0;
        cotunnel[i].id = i;
        lotunnel[i].id = i;
        cotunnel[i].buf = (byte*)malloc(sizeof(byte) * BUFSIZE);
        lotunnel[i].buf = (byte*)malloc(sizeof(byte) * BUFSIZE);
    }

    struct ev_io socket_accept;

    get_param(argc, argv);

//    options.faddr = "0.0.0.0:8765";
//    options.baddr = "";
//    options.cryptoMethod = "RC4";
//    options.secret = "secret";
//    options.clientMod = 1;

    build_server();

    ev_io_init(&socket_accept, accept_cb, serv_sock, EV_READ);
    ev_io_start(loop, &socket_accept);

    while(1) {
        ev_loop(loop, 0);
    }

    return 0;
}

void local_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {

    if(EV_ERROR & revents)
    {
      printf("error event in read");
      return;
    }
    //puts("remote_cb");
    //printf("size == %d\n", sizeof(coinput[pos]));
    struct tunnel_ctx* tunnel = (struct tunnel_ctx*)watcher;
    int count = recv(tunnel->fd, tunnel->buf, BUFSIZE, 0);
    int pos = tunnel->id;
    if(count < 0) {
        perror("error count");
        ev_io_stop(loop, watcher);
        ev_io_stop(loop, &(cotunnel[pos].io));
        close(lotunnel[pos].fd);
        close(cotunnel[pos].fd);
        isused[pos] = 0;
        return ;
    }
    if(count == 0) {
        ev_io_stop(loop, watcher);
        ev_io_stop(loop, &(cotunnel[pos].io));
        close(lotunnel[pos].fd);
        close(cotunnel[pos].fd);
        isused[pos] = 0;
        return ;
    }
    printf("log : read %d byte from client\n", count);

    //memset(buffer2, 0, sizeof(buffer2));
    RC4(&tunnel->key, count, tunnel->buf, tunnel->buf);

    send(cotunnel[tunnel->id].fd, tunnel->buf, count, 0);
    puts("send ok");
}

void remote_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {

    if(EV_ERROR & revents)
    {
      printf("error event in read");
      return;
    }

    //puts("remote_cb");
    struct tunnel_ctx* tunnel = (struct tunnel_ctx*)watcher;
    int count = recv(tunnel->fd, tunnel->buf, BUFSIZE, 0);
    int pos = tunnel->id;
    if(count < 0) {
        perror("error count");
        ev_io_stop(loop, watcher);
        ev_io_stop(loop, &(lotunnel[pos].io));
        close(lotunnel[pos].fd);
        close(cotunnel[pos].fd);
        isused[pos] = 0;
        return ;
    }
    if(count == 0) {
        ev_io_stop(loop, watcher);
        ev_io_stop(loop, &(lotunnel[pos].io));
        close(lotunnel[pos].fd);
        close(cotunnel[pos].fd);
        isused[pos] = 0;
        return ;
    }
    printf("log : read %d byte from remote\n", count);

    //memset(buffer2, 0, sizeof(buffer2));
    RC4(&tunnel->key, count, tunnel->buf, tunnel->buf);

    send(lotunnel[tunnel->id].fd, tunnel->buf, count, 0);
    puts("send ok");
}

void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    int nfd, i, remote_sock, j, o, flags;
    int clnt_adr_size;
    struct sockaddr_in addr, remote_adr;
    nfd = accept(serv_sock, (struct sockaddr*) &addr, &clnt_adr_size);

    //printf("nfd == %d\n", nfd);

    if(nfd == -1) return ;
    j = 1;
    //ioctl(nfd, FIONBIO, &j);
    flags = fcntl(nfd, F_GETFL, 0);
    fcntl(nfd, F_SETFL, flags | O_NONBLOCK);
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
        if (!SAFE_REALLOC(&isused, sizeof(int) * o, sizeof(int) * colen)) {
			goto Shortage;
		}
        if (!SAFE_REALLOC(&lotunnel, sizeof(struct tunnel_ctx) * o, sizeof(struct tunnel_ctx) * colen)) {
			goto Shortage;
		}
        if (!SAFE_REALLOC(&cotunnel, sizeof(struct tunnel_ctx) * o, sizeof(struct tunnel_ctx) * colen)) {
			goto Shortage;
		}

        for(i = o; i < colen ; ++i) {
            isused[i] = 0;
            lotunnel[i].buf = (byte *)malloc(sizeof(byte) * BUFSIZE);
            cotunnel[i].buf = (byte *)malloc(sizeof(byte) * BUFSIZE);
            lotunnel[i].id = i;
            cotunnel[i].id = i;
        }
        pos = o;
    }


    if(pos != -1) {
        //printf("pos ====================================================================================== %d\n", pos);
        isused[pos] = 1;

        lotunnel[pos].fd = nfd;

        memset(&remote_adr, 0, sizeof(remote_adr));
        remote_adr.sin_family = AF_INET;
        remote_adr.sin_port = htons(atoi(setting.baddr_port));
        remote_adr.sin_addr.s_addr = inet_addr(setting.baddr_host);

        remote_sock = socket(PF_INET, SOCK_STREAM, 0);
        //printf("socks == %d   |   %d\n", nfd, remote_sock);

        if(remote_sock < 0) {
            perror("socket error");
            isused[pos] = 0;
            close(nfd);
            return ;
        }

        j = 0;
        setsockopt(remote_sock, SOL_SOCKET, SO_LINGER, &j, sizeof(j));

        cotunnel[pos].fd = remote_sock;

        if ( connect(remote_sock, (struct sockaddr *) &remote_adr, sizeof(remote_adr)) < 0) {
            perror("connect remote error");
            exit(1);
        }

        j = 1;
	    //ioctl(remote_sock, FIONBIO, &j);
        flags = fcntl(remote_sock, F_GETFL, 0);
        fcntl(remote_sock, F_SETFL, flags | O_NONBLOCK);

        RC4_set_key(&(lotunnel[pos].key), 16, setting.secret);
        RC4_set_key(&(cotunnel[pos].key), 16, setting.secret);

        ev_io_init(&(lotunnel[pos].io), local_cb, nfd, EV_READ);
        ev_io_init(&(cotunnel[pos].io), remote_cb, remote_sock, EV_READ);
        ev_io_start(loop, &(lotunnel[pos].io));
        ev_io_start(loop, &(cotunnel[pos].io));
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


    if(serv_sock < 0) {
        perror("socket error");
        exit(1);
    }

    int optval = 1;
    if(setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt error");
        exit(1);
    }
    int flags = fcntl(serv_sock, F_GETFL, 0);
    fcntl(serv_sock, F_SETFL, flags | O_NONBLOCK);
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
