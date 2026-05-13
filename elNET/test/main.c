#include "plat.h"

#if defined(ELNET_PLAT_WINDOWS)
#include <winsock2.h>
#elif defined(ELNET_PLAT_LINUX)
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#elif defined(ELNET_PLAT_ELOS)

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error_handling(char * message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

char message[100] = {1,2,3,4,6,7,9,0,12};

int main(char argc, char * argv[])
{
    int serv_sock;
    struct sockaddr_in serv_adr;

    serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if( serv_sock == -1 )
        error_handling("socket create err");

    memset(&serv_adr, 0, sizeof(struct sockaddr_in));

    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if( bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1 )
        error_handling("bind error");
    
    int str_len = 20;
    struct sockaddr_in clnt_adr;
    int clnt_adr_sz = sizeof(struct sockaddr);
    while(1)
    {
        str_len = recvfrom(serv_sock, message, 100, 0, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
        printf("recv\r\n");
        sendto(serv_sock, message, str_len, 0, (struct sockaddr *)&clnt_adr, clnt_adr_sz);
    }

    close(serv_sock);
    return 0;
}