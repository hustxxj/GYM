#include "widget.h"
#include "ui_widget.h"
//PAD02

static QTimer *timer=new QTimer;  //线程1,每隔50ms显示一帧人脸图像到屏幕上，并进行一次人脸检测
static QTimer *timer_connect=new QTimer; //线程2,每隔50ms访问一次临界区，看是否有人脸数据到达，如果有则发送出去

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowTitle("人脸识别02号机");
    setFixedSize(this->width(),this->height());

    net=cv::dnn::experimental_dnn_34_v7::readNetFromCaffe(modelConfiguration, modelBinary);
    dlib::deserialize("./Face/dlib_face_recognition_resnet_model_v1.dat") >> nets;
//    cap.open(0);
//    if(!cap.isOpened())
//    {
//        std::cout<<"camera open fail"<<std::endl;
//        exit(1);
//    }

    visitfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&addr_visit,sizeof(addr_visit));
    addr_visit.sin_family=AF_INET;
    addr_visit.sin_port=htons(10000);
    inet_pton(AF_INET,"127.0.0.1",&addr_visit.sin_addr.s_addr);
    int retinfo=::connect(visitfd,(sockaddr*)&addr_visit,sizeof(addr_visit));
    if(retinfo==-1)
    {
        std::cout<<"pad connect server fail"<<std::endl;
        exit(1);
    }else{
        std::cout<<"pad connect success"<<std::endl;
        char tempbuf[128];
        char padnum[]="_PAD02";
        sprintf(tempbuf,"%s",padnum);
        write(visitfd,tempbuf,strlen(tempbuf));
    }

    timer->start(5000);
    connect(timer,SIGNAL(timeout()),this,SLOT(playImg()));
    timer_connect->start(500);
    connect(timer_connect,SIGNAL(timeout()),this,SLOT(sendFrame()));

}

Widget::~Widget()
{
    delete ui;
}

/*读入图片进行人脸检测，并显示在屏幕上，设置每50ms读取一帧*/
void Widget::playImg()
{
//    cap>>srcFrame;
    srcFrame = cv::imread("./20200528100203.jpg");
    if (srcFrame.channels() == 4)
        cv::cvtColor(srcFrame, srcFrame, cv::COLOR_BGRA2BGR);
    cv::Mat inputBlob = cv::dnn::blobFromImage(srcFrame, inScaleFactor,
                cv::Size(inWidth, inHeight), meanVal, false, false);
    net.setInput(inputBlob, "data");
    cv::Mat detection = net.forward("detection_out");

    std::vector<double> layersTimings;
    double freq = cv::getTickFrequency() / 1000;//用于返回CPU的频率。get Tick Frequency。这里的单位是秒，也就是一秒内重复的次数。
    double time = net.getPerfProfile(layersTimings) / freq;
    std::ostringstream ss;
    ss << "FPS: " << 1000 / time << " ; time: " << time << " ms";
    cv::putText(srcFrame, ss.str(), cv::Point(20, 20), 0, 0.5, cv::Scalar(0, 0, 255));

    cv::Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());//101*7矩阵
    for (int i = 0; i < detectionMat.rows; i++)
    {
        float confidence = detectionMat.at<float>(i, 2);      //第二列存放可信度

        if (confidence > min_confidence)//满足阈值条件
        {
            //存放人脸所在的图像中的位置信息
            int xLeftBottom = static_cast<int>(detectionMat.at<float>(i, 3) * srcFrame.cols);
            int yLeftBottom = static_cast<int>(detectionMat.at<float>(i, 4) * srcFrame.rows)+50;
            int xRightTop = static_cast<int>(detectionMat.at<float>(i, 5) * srcFrame.cols);
            int yRightTop = static_cast<int>(detectionMat.at<float>(i, 6) * srcFrame.rows);

            cv::Rect object((int)xLeftBottom, (int)yLeftBottom,//定义一个矩形区域（x,y,w,h)
                (int)(xRightTop - xLeftBottom),
                (int)(yRightTop - yLeftBottom));
            if (object.width < 50 || object.height < 50)
                continue;
            //检测到人脸则将人脸存入临界区，并保证临界区只留存当前最新的一帧人脸
            if(saveMat.empty())
            {
                saveMat.push_back(srcFrame);
            }else{
                std::vector<cv::Mat>().swap(saveMat);
                saveMat.push_back(srcFrame);
            }
            cv::rectangle(srcFrame, object, cv::Scalar(0, 255, 0));//画个人脸框
            break;
        }else{
            std::vector<cv::Mat>().swap(saveMat);
        }
    }

    cv::resize(srcFrame, srcFrame, cv::Size(ui->imglabel->width(), ui->imglabel->height()));
    cv::cvtColor(srcFrame, srcFrame, CV_BGR2RGB);
    QImage imgQ(srcFrame.data, srcFrame.cols, srcFrame.rows, srcFrame.step, QImage::Format_RGB888);
    ui->imglabel->setPixmap(QPixmap::fromImage(imgQ));
}

void Widget::sendFrame()
{
    if(!saveMat.empty())
    {
        cv::Mat frame;
        frame=saveMat[0];
        std::vector<cv::Mat>().swap(saveMat); //清空save，避免发送线程多次获得锁造成一张人脸发送多次
        /*
         * 如果直接将frame转成unsigned char* 传输，这样虽然方便，但是在接受方只能将unsigned char*数据转换成黑白图片，
         * 而人脸识别需要的frame是rgb三维的
        */
        IplImage imgTmp = frame;
        IplImage *input = cvCloneImage(&imgTmp);
        int imgbufsize=1024;
        sprintf(buf,"width:%d,height:%d,depth:%d,channel:%d,imgbufsize:%d,INFOEND",
                input->width,input->height,input->depth,input->nChannels,imgbufsize);
        write(visitfd,buf,sizeof(buf));

        int num=0,ret;
        while(num<input->imageSize-imgbufsize)
        {
            ret=write(visitfd,(char*)(input->imageData+num),imgbufsize);
            num+=ret;
            std::cout<<"[SENT]"<<num<<std::endl;
        }
        write(visitfd,(char*)input->imageData+num,input->imageSize-num);

        read(visitfd,buf,sizeof(buf));
        std::string retID=buf;
        std::cout<<"[FROM SERVER]"<<retID.substr(0,14)<<std::endl;
        if(retID.substr(0,14)=="00000000000000")
        {
            std::cout<<"人脸验证失败"<<std::endl;
            return;
        }else{
            std::cout<<"会员："<<retID.substr(0,14)<<",欢迎光临"<<std::endl;

//            QString text=QString::fromStdString("会员:"+retID);
//            QMessageBox::warning(this,tr("欢迎光临"),text);

            timer->stop();
            timer_connect->stop();

            read(visitfd,buf,sizeof(buf));

            timer->start(50);
            timer_connect->start(500);
        }
    }
}
