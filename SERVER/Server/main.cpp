#include <QCoreApplication>
#include "main.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    cv::String dir="./Face/FaceStock/";
    bool isload=idrec->LoadFaceStock(dir);
    if(isload==false)
    {
        cout<<"加载人脸库失败!"<<endl;
        return -1;
    }else{
        cout<<"加载人脸库成功!"<<endl;
    }

    struct sockaddr_in serv_addr;
    socklen_t serv_len=sizeof(serv_addr);

    //创建套接字
    int lfd=socket(AF_INET,SOCK_STREAM,0);
    memset(&serv_addr,0,serv_len);
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(10000);
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    int opt = 1;
    //设置端口复用
    setsockopt(lfd, SOL_SOCKET,SO_REUSEADDR, (const void *)&opt, sizeof(opt) );
    //绑定IP和端口
    bind(lfd,(struct sockaddr*)&serv_addr,serv_len);
    //设置同时监听的最大个数
    listen(lfd,36);
    printf("启动监听......\n");

    struct sockaddr_in client_addr;
    socklen_t cli_len=sizeof(client_addr);

    //最大文件描述符
    int maxfd=lfd;
    //文件描述符读集合
    fd_set reads,temp;
    //reads初始化
    FD_ZERO(&reads);
    FD_SET(lfd,&reads);
    while(1)
    {
        pthread_mutex_lock(&mutexs);
        //委托内核做IO检测
        temp=reads;
        int ret=select(maxfd+1,&temp,NULL,NULL,NULL);
        if(ret == -1)
        {
            perror("select error");
            exit(1);
        }
        //有客户端发起了新的连接
        if(FD_ISSET(lfd,&temp))
        {
            /*
            *6位符号用于传送设备信息
            *10位用于与闸机通信
            *14位用于与PAD通信
            */

            //接受链接请求 - accept不阻塞
            int cfd=accept(lfd,(struct sockaddr*)&client_addr,&cli_len);
            if(cfd == -1)
            {
                perror("accept error");
                exit(1);
            }
            char ip[6];
            printf("new client IP: %s,Port:%d\n",
                             inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,ip,sizeof(ip)),
                             ntohs(client_addr.sin_port));
            //设置cfd为非阻塞
            int flag=fcntl(cfd,F_GETFL);
            flag |= O_NONBLOCK;
            fcntl(cfd,F_SETFL,flag);

            //将cfd加入到待检测的读集合中-下一次就可以检测到了
            FD_SET(cfd,&reads);
            //更新最大文件描述符
            maxfd=(maxfd<cfd)?cfd:maxfd;
        }
        for(int i=lfd+1;i<=maxfd;i++)
        {
            if(FD_ISSET(i,&temp))
            {
                char buf[4096]={0};
                int recv_len=read(i,buf,sizeof(buf));
                std::cout<<"[RECV LEN]:"<<recv_len<<endl;
                if(recv_len==6)  //收到的是客户端类型和编号
                {
                    string str=buf;
                    std::cout<<"客户端类型和编号. :"<<str<<std::endl;
                    string type_str=str.substr(0,4);
                    string num_str=str.substr(4,6);
                    if(type_str=="_PAD")
                    {
                        vec[0][atoi(num_str.c_str())]=i;
                        cout<<"FD _PAD:"<<i<<endl;
                    }else if(type_str=="DOOR")
                    {
                        vec[1][atoi(num_str.c_str())]=i;
                        cout<<"FD DESK:"<<i<<endl;
                    }else if(type_str=="DESK")
                    {
                        vec[2][atoi(num_str.c_str())]=i;
                    }
                }
                else if(recv_len==10) //收到的是闸机的信号
                {
                    string str=buf;
                    if(str!="0000000000")//验证闸机回传的信号是否是关闸信号
                    {
                        continue;
                    }
                    int door_num=0;
                    for(int k=0;k<vec[1].size();k++)
                    {
                        if(vec[1][k]==i)
                        {
                            door_num=k;
                            break;
                        }
                    }
                    write(vec[0][door_num],"0000",4);  //给人脸识别端发送关闸信号
                }
                else  //收到人脸识别端的人脸数据
                {
                    int _pad_num=0;   //平板编号
                    for(int k=0;k<vec[0].size();k++)
                    {
                        if(vec[0][k]==i)
                        {
                            _pad_num=k;
                            break;
                        }
                    }

                    string str=buf;
                    cout<<str<<endl;
                    string imginfo[5];
                    int imginfo_ptr=0;
                    for(int k=0;i<strlen(buf);k++)
                    {
                        if(str[k]==':')
                        {
                            k++;
                            while(str[k]!=',')
                            {
                                imginfo[imginfo_ptr].push_back(str[k]);
                                k++;
                            }
                            imginfo_ptr++;
                            if(imginfo_ptr>4)
                                break;
                        }
                    }

                    //接收IplImage格式图片数据
                    IplImage *img;
                    int width,height,depth,channels,buffersize;

                    width=atoi(imginfo[0].c_str());
                    height=atoi(imginfo[1].c_str());
                    depth=atoi(imginfo[2].c_str());
                    channels=atoi(imginfo[3].c_str());
                    buffersize=atoi(imginfo[4].c_str());
                    cout<<width<<","<<height<<","<<depth<<","<<channels<<","<<buffersize<<endl;

                    img=cvCreateImage(cvSize(width,height),depth,channels);
                    int imsize=img->imageSize;
                    int imgnum=0,ret;

                    while (imgnum<img->imageSize-buffersize) {
                        ret=read(i,(char*)img->imageData+imgnum,buffersize);
                        imgnum+=ret;
                        cout<<"[RECV SIZE]"<<imgnum<<";[RET]"<<ret<<endl;
                    }
                    read(i,(char*)(img->imageData+imgnum),img->imageSize-imgnum);

                    cout<<imsize<<"-"<<imgnum<<endl;

                    Mat srcframe=cvarrToMat(img);
                    string ID="";
                    ID=idrec->FaceRecognize(srcframe,0.4);
                    if(ID==idrec->ErrorType1 || ID==idrec->ErrorType2)
                    {
                        //发送不是会员给平板端
                        std::cout<<"[平板-"<<_pad_num<<"]:识别失败"<<std::endl;
                        bzero(buf,sizeof(buf));
                        write(i,"00000000000000",14);  //"00000000000000"表示不是会员，或识别失败
                    }else
                    {
                       ID=ID.substr(ID.size()-18,14);
                       // 1.发送会员号给平板端
                       write(i,ID.c_str(),strlen(ID.c_str()));
                       // 2.发送开闸信号给闸机
                       cout<<"FD SENDDOOR:"<<vec[1][_pad_num]<<endl;
                       write(vec[1][_pad_num],"1111111111",10);
                       // 3.发送会员号给客户端
                    }
                }
            }
        }
        pthread_mutex_unlock(&mutexs);
    }

    return a.exec();
}
