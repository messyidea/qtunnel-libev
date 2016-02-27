CC = gcc
CFLAGS = -g -w
SRCS = qtunnel.c qtunnel.h

all: qtunnel

qtunnel: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o qtunnel -lcrypto -lpthread

clean:
	rm -f qtunnel
