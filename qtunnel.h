#ifndef QTUNNEL_H
#define QTUNNEL_H

struct struct_options {
    char *faddr;
    char *baddr;
    char *cryptoMethod;
    char *secret;
    char *logTo;
    int clientMod;
};

struct tunnel {
    int server_socket;
    int client_socket;
    int remote_socket;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    struct sockaddr_in remote_addr;
};


#endif
