# qtunnel-c

[qtunnel]("https://github.com/getqujing/qtunnel") in c

As qtunnel can not run on my loongson 8089d (alloc memory error, gccgo), I write this.

Because of lacking knowledge of openssl and aes, it could only support rc4 now.

it can work with [qtunnel]("https://github.com/getqujing/qtunnel") in go.


usage
---
```
server: ./qtunnel --listen=0.0.0.0:server_port --backend=127.0.0.1:7655 --clientmod=false --secret=secret

client: ./qtunnel --listen=127.0.0.1:local_port --backend=server_host:server_port --clientmod=true --secret=secret
```
