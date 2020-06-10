#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include<string>

int main(int argc,char* argv[])
{
    if(argc<2)
    {
        printf("./a.out doornum\n");
        return 0;
    }

    int serverfd;
    serverfd=socket(AF_INET,SOCK_STREAM,0);

    sockaddr_in addr_server;
    bzero(&addr_server,sizeof(addr_server));
    addr_server.sin_family=AF_INET;
    addr_server.sin_port=htons(10000);
    inet_pton(AF_INET,"127.0.0.1",&addr_server.sin_addr.s_addr);

    connect(serverfd,(sockaddr*)&addr_server,sizeof(addr_server));

    //0000000000--闸机close，1111111111--闸机open
    char buf[128];
    sprintf(buf,"%s",argv[1]);
    write(serverfd,buf,strlen(buf));

    while(1)
    {
        read(serverfd,buf,sizeof(buf));
        std::string retbuf=buf;
        retbuf=retbuf.substr(0,10);
        if(retbuf=="1111111111")
        {
            printf("闸机已打开\nSLEEP-模拟用户通过闸机\n");
            sleep(10);//模拟开关闸机
            printf("闸机已关闭\n");
        }else{
            continue;
        }
        write(serverfd,"0000000000",10);
        printf("关闸信号已发送\n");
    }

    return 0;
}
