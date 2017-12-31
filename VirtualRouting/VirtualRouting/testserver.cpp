#include <stdio.h>
#include <WinSock2.h>


int main()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(1,1);

    err = WSAStartup(wVersionRequested,&wsaData);

    SOCKET sockSrv = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.s_addr = inet_addr("127.000.000.001");
    addrSrv.sin_family=AF_INET;
    addrSrv.sin_port=htons(8080);
     bind(sockSrv,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));


     SOCKADDR_IN addrClient;
     int len=sizeof(SOCKADDR);

     while(1)
     {
         char recvBuf[20];
         int ret = recvfrom(sockSrv, recvBuf, 20, 0, (SOCKADDR*)&addrClient, &len);
         if (ret > 0 ) {
            printf("Yes\n");
            printf("%s\n",recvBuf);
         }
         char response[] = "I have received";
         sendto(sockSrv, response, strlen(response), 0, (SOCKADDR*)&addrClient, len);
   }
    return 0;
}

// g++ testserver.cpp -o server -lws2_32