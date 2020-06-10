#include "IDRec.h"


IDRec::IDRec():
    vec(),
    fileNames()
{
    detector = get_frontal_face_detector();
    deserialize("./Face/shape_predictor_68_face_landmarks.dat") >> sp;
    deserialize("./Face/dlib_face_recognition_resnet_model_v1.dat") >> net;
    nets = cv::dnn::experimental_dnn_34_v7::readNetFromCaffe(modelConfiguration, modelBinary);
}


bool IDRec::LoadFaceStock(cv::String facestocksdir)
{
    vec.clear();
    cv::String dir_path = facestocksdir;
    cv::glob(dir_path, fileNames);                     //遍历人脸库
    if (fileNames.size() == 0)
    {
        cout << "Face stock is empty!" << endl;
        return false;
    }
    matrix<rgb_pixel> dlibtype_img;
    for (int i = 0; i < fileNames.size(); i++)
    {
        string fileFullName = fileNames[i];
        load_image(dlibtype_img,fileFullName);
        std::vector<dlib::rectangle>dets = detector(dlibtype_img);
        if (dets.size() != 1)
        {
            cout << "Reading Face stock error,please check: " << fileNames[i] << endl;
            return false;
        }
        std::vector<matrix<rgb_pixel>> faces;
        auto shape = sp(dlibtype_img,dets[0]);
        matrix<rgb_pixel> face_chip;
        extract_image_chip(dlibtype_img,get_face_chip_details(shape,150,0.25),face_chip);//尺度归一化为150x150大小
        faces.push_back(move(face_chip));
        std::vector<matrix<float, 0, 1>> face_descriptors = net(faces);//将图片送入resnet残差网络转换为128维编码特征，存入vec中备用
        vec.push_back(face_descriptors[0]);
        cout << "The vector of picture " << fileNames[i] << "is:" << trans(face_descriptors[0]) << endl;//打印人脸ID
    }
    return true;
}

string IDRec::FaceRecognize(Mat frame_img, double error_min)
{
    string out;
    std::vector<float> vec_error;                                    //存放人脸与库中每个脸比对后的相似的差值
    matrix<rgb_pixel> dlibtype_img;
    assign_image(dlibtype_img,cv_image<rgb_pixel>(frame_img));
    std::vector<matrix<rgb_pixel>> faces_test;
    for (auto face_test : detector(dlibtype_img))
    {
        auto shape_test = sp(dlibtype_img,face_test);
        matrix<rgb_pixel> face_chip_test;
        extract_image_chip(dlibtype_img,get_face_chip_details(shape_test, 150, 0.25),face_chip_test);
        faces_test.push_back(move(face_chip_test));
    }
    if (faces_test.size() != 1)
    {
        cout << "camera disturb!Please Maintain good posture and Exclude Unrelated personnel!" << endl;
        return ErrorType1;
    }
    std::vector<matrix<float, 0, 1>> face_test_descriptors = net(faces_test);
    for (size_t i = 0; i < face_test_descriptors.size(); ++i)
    {
        size_t m = 100;
        float v_error = 100.0;
        for (size_t j = 0; j < vec.size(); ++j)
        {
            vec_error.push_back((double)length(face_test_descriptors[i]-vec[j]));
            if (vec_error[j] < v_error)
            {
                v_error = vec_error[j];
                m = j;
            }
        }
        if (v_error < error_min)
        {
            out = fileNames[m];
        }
        else
        {
            out = ErrorType2;
        }
    }
    return out ;
}
