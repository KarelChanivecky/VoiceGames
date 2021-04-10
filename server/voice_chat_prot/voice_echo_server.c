/**
* karel on 2021-04-09.
*/

#include "voice_prot.h"
#include "stdio.h"

int main() {
    int sock = get_voice_sock();
    puts("port open");
    while (123) {

        struct sockaddr client;
        datagram_t datagram;
        recv_voice(sock, &datagram, &client);
        puts("received");
        send_voice(sock, &datagram, &client);
        puts("sent\n");
    }

}
