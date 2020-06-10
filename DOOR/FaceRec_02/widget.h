#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <QMessageBox>
#include <QTimer>
#include <vector>
#include <sys/io.h>
#include <utility>
#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include <ctype.h>
#include <pthread.h>
#include <opencv2/dnn/dnn.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>
#include <dlib/image_io.h>
#include <dlib/string.h>
#include <dlib/dnn.h>
#include <dlib/gui_widgets.h>
#include <dlib/clustering.h>
#include <dlib/image_loader/image_loader.h>


namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

    template <template <int, template<typename>class, int, typename> class block, int N, template<typename>class BN, typename SUBNET>
    using residual = dlib::add_prev1<block<N, BN, 1, dlib::tag1<SUBNET>>>;

    template <template <int, template<typename>class, int, typename> class block, int N, template<typename>class BN, typename SUBNET>
    using residual_down = dlib::add_prev2<dlib::avg_pool<2, 2, 2, 2, dlib::skip1<dlib::tag2<block<N, BN, 2, dlib::tag1<SUBNET>>>>>>;

    template <int N, template <typename> class BN, int stride, typename SUBNET>
    using block = BN<dlib::con<N, 3, 3, 1, 1, dlib::relu<BN<dlib::con<N, 3, 3, stride, stride, SUBNET>>>>>;

    template <int N, typename SUBNET> using ares = dlib::relu<residual<block, N, dlib::affine, SUBNET>>;
    template <int N, typename SUBNET> using ares_down = dlib::relu<residual_down<block, N, dlib::affine, SUBNET>>;

    template <typename SUBNET> using alevel0 = ares_down<256, SUBNET>;
    template <typename SUBNET> using alevel1 = ares<256, ares<256, ares_down<256, SUBNET>>>;
    template <typename SUBNET> using alevel2 = ares<128, ares<128, ares_down<128, SUBNET>>>;
    template <typename SUBNET> using alevel3 = ares<64, ares<64, ares<64, ares_down<64, SUBNET>>>>;
    template <typename SUBNET> using alevel4 = ares<32, ares<32, ares<32, SUBNET>>>;

    using anet_type = dlib::loss_metric<dlib::fc_no_bias<128, dlib::avg_pool_everything<
        alevel0<
        alevel1<
        alevel2<
        alevel3<
        alevel4<
        dlib::max_pool<3, 3, 2, 2, dlib::relu<dlib::affine<dlib::con<32, 7, 7, 2, 2,
        dlib::input_rgb_image_sized<150>
        >>>>>>>>>>>>;

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

    bool LoadFaceStock(const cv::String& facestocksdir);    //加载人脸数据库

private slots:
    void playImg();
    void sendFrame();

private:
    Ui::Widget *ui;

    cv::VideoCapture cap;
    cv::Mat srcFrame;

    //加载人脸检测网络
    cv::String modelConfiguration = "./Face/deploy.prototxt";
    cv::String modelBinary = "./Face/res10_300x300_ssd_iter_140000_fp16.caffemodel";
    cv::dnn::Net net;
    anet_type nets;

    //人脸检测参数
    const size_t inWidth = 300;
    const size_t inHeight = 300;
    const double inScaleFactor = 1.0;
    const cv::Scalar meanVal=(104.0,117.0,123.0);
    float min_confidence = 0.99;

    //临界区，存储人脸
    std::vector<cv::Mat> saveMat;
    pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

    //网络通信模块
    int visitfd;
    sockaddr_in addr_visit;
    char buf[4096]={0};
};

#endif // WIDGET_H
