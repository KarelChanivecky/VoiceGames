/**
* karel on 2021-04-09.
*/

#include "voice_prot.h"
#include "stdio.h"

int main() {
    int sock = get_inbound_sock();
    puts("tcp_port open");
    while (123) {

        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        datagram_t datagram;
        recv_voice( sock, &datagram, ( struct sockaddr * ) &client, &client_len);
//        printf("client tcp_port: %d", client.sin_port);
//        puts("received");

        char str[INET_ADDRSTRLEN + 1];
        inet_ntop(AF_INET, &client.sin_addr, str, INET_ADDRSTRLEN);

        printf("%d\n", client.sin_family);
        printf("%d\n", ntohs(client.sin_port));
        printf("%s\n", str);


        send_voice( sock, &datagram, ( struct sockaddr * ) &client );
//        puts("sent\n");

    }

}
