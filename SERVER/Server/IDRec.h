/* 
     日期：2019.11.22  
     作者：华中科技大学--许学杰
     说明：该类包含人脸检测，人脸关键点获取，人脸识别等与人脸相关函数
*/

#include <cstdio>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>

#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <opencv2/core.hpp>

#include <dlib/dnn.h>
#include <dlib/gui_widgets.h>
#include <dlib/clustering.h>
#include <dlib/string.h>
#include <dlib/image_io.h>
#include <dlib/image_loader/jpeg_loader.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>


using namespace dlib;
using namespace std;
using namespace cv;

struct FACEDTK {
	bool result;
    std::vector<cv::Point2f> shape;
};

class IDRec {

	template <template <int, template<typename>class, int, typename> class block, int N, template<typename>class BN, typename SUBNET>
	using residual = add_prev1<block<N, BN, 1, tag1<SUBNET>>>;

	template <template <int, template<typename>class, int, typename> class block, int N, template<typename>class BN, typename SUBNET>
	using residual_down = add_prev2<avg_pool<2, 2, 2, 2, skip1<tag2<block<N, BN, 2, tag1<SUBNET>>>>>>;

	template <int N, template <typename> class BN, int stride, typename SUBNET>
	using block = BN<con<N, 3, 3, 1, 1, relu<BN<con<N, 3, 3, stride, stride, SUBNET>>>>>;

	template <int N, typename SUBNET> using ares = relu<residual<block, N, affine, SUBNET>>;
	template <int N, typename SUBNET> using ares_down = relu<residual_down<block, N, affine, SUBNET>>;

	template <typename SUBNET> using alevel0 = ares_down<256, SUBNET>;
	template <typename SUBNET> using alevel1 = ares<256, ares<256, ares_down<256, SUBNET>>>;
	template <typename SUBNET> using alevel2 = ares<128, ares<128, ares_down<128, SUBNET>>>;
	template <typename SUBNET> using alevel3 = ares<64, ares<64, ares<64, ares_down<64, SUBNET>>>>;
	template <typename SUBNET> using alevel4 = ares<32, ares<32, ares<32, SUBNET>>>;

	using anet_type = loss_metric<fc_no_bias<128, avg_pool_everything<
		alevel0<
		alevel1<
		alevel2<
		alevel3<
		alevel4<
		max_pool<3, 3, 2, 2, relu<affine<con<32, 7, 7, 2, 2,
		input_rgb_image_sized<150>
		>>>>>>>>>>>>;                                   //定义一个ResNet网络

public:
	IDRec();
    ~IDRec()
    {

    }
    bool LoadFaceStock(cv::String facestocksdir);    //加载人脸数据库
	string FaceRecognize(Mat frame_img, double error_min);   //进行人脸识别

public:
	const string ErrorType1 = "FaceDisturb";
    const string ErrorType2 = "NotFound";

    cv::Mat test_frame;


private:
	frontal_face_detector detector;
	shape_predictor sp;
	anet_type net;
	std::vector<matrix<float, 0, 1>> vec;        //定义一个向量组，用于存放每一个人脸的编码
	std::vector<cv::String> fileNames;

	cv::dnn::Net nets;
    cv::String modelConfiguration = "./Face/deploy.prototxt";
    cv::String modelBinary = "./Face/res10_300x300_ssd_iter_140000_fp16.caffemodel";
};


