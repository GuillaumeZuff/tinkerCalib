#include "calibration.h"
#include "posConverter.h"
#include <iostream>

using namespace cv;
using namespace std;

Calibration::Calibration()
{
}

Calibration::~Calibration()
{
}

void printMatrix(const std::string &name, const cv::Mat &mat) {
    cout<<name<<":"<<endl<<"["<<endl;
    for (int i=0; i<mat.rows;i++) {
        for (int j=0; j<mat.cols; j++) {
            cout<<mat.at<float>(j,i);
            if (j<mat.cols-1)
                cout<<",";
        }
        cout<<endl;
    }
    cout<<"]"<<endl;
}

void print(const std::string &name, const std::vector<float> &v) {
    cout << name<<":"<<endl<<"["<<endl;
    for (int i=0; i<4; i++) {
        for (int j=0; j<4; j++) {
            cout<<v[i*4+j];
            if (j<3) 
                cout<<",";
        }
        cout<<endl;
    }
    cout<<"]"<<endl;
}

void Calibration::testConversion(std::vector<cv::Point2f> &mmPoints,
        std::vector<cv::Point2f> &pxPoints) {
    PosConverter pc;
    pc.setIntrinsic(m_intrinsic);
    pc.setRotation(m_rotation);
    pc.setTranslation(m_translation);
    Point2f res = pc.mm2pixel(mmPoints[0]);
    cout << "Testing calib: "<<endl;
    cout << "Converting "<<mmPoints[0].x<<" "<<mmPoints[0].y;
    cout << " to "<<pxPoints[0].x<<" "<<pxPoints[1].y<<endl;
    cout << "Result: "<<res.x<<" "<<res.y<<endl;
    Point2f res2 = pc.pixel2mm(pxPoints[0]);
    cout << "Now px to mm: "<<res2.x<<" "<<res2.y<<endl;
}

bool Calibration::calibrateFromPoints(std::vector<float> imagePoints, std::vector<float> objectPoints, bool isCamera)
{
    vector<Point2f> camPoints, realPoints;
    for (int i=0; i<(int)imagePoints.size(); i+=2) {
        camPoints.push_back(Point2f(imagePoints[i],imagePoints[i+1]));
        if (isCamera)
            realPoints.push_back(Point2f(objectPoints[i],objectPoints[i+1]));
        else
            realPoints.push_back(Point2f(objectPoints[i],-objectPoints[i+1]));
    }
    return doCalibration(camPoints,realPoints,isCamera);
}

bool Calibration::doCalibration(const std::vector<Point2f> &camPoints, const std::vector<Point2f> &realPoints, bool isCamera) {
    vector<Point2f> imagePoints = camPoints;
    vector<Point2f> objectPoints = realPoints;
    std::vector<cv::Point3f> objectPoints3d(objectPoints.size());
    for (int i = 0; i < (int)objectPoints3d.size(); i++)
    {
        objectPoints3d[i].x = objectPoints[i].x;
        objectPoints3d[i].y = objectPoints[i].y;
        objectPoints3d[i].z = 0.f;
        if (!isCamera) {
            imagePoints[i].x = m_calibData.frameSize.width - imagePoints[i].x;
        }
        //imagePoints[i].y = m_calibData.frameSize.height - imagePoints[i].y;
        //cout<<'['<<imagePoints[i].x<<","
        //<<imagePoints[i].y<<","<<
        //m_realMmPoints[i].x<<","<<m_realMmPoints[i].y<<"]"<<endl;
    }
    Mat rotVector, tmpRotation, tmpTranslation;
    solvePnP(Mat(objectPoints3d), Mat(imagePoints),
            m_intrinsic, m_distortion, rotVector, tmpTranslation);
    Rodrigues(rotVector, tmpRotation);

    tmpRotation.convertTo(m_rotation, CV_32F);
    tmpTranslation.convertTo(m_translation, CV_32F);

    // this is important - we invert the translation vector
    if (isCamera) {
        m_translation = m_translation*-1;
    }

    //printMatrix("intrinsic", m_intrinsic);
    //printMatrix("rotation", m_rotation);
    //printMatrix("translation", m_translation);
    //print("projection",getProjectionMatrix(isCamera));
    //print("modelview",getMvMatrix(isCamera));

    //testConversion(objectPoints,imagePoints);

    return true;
}

bool Calibration::calibrate(const std::string &file,bool isCamera)
{
    //cout<<"Camera? "<<isCamera<<endl;
    cout<<"File"<<file<<endl;
    Mat frame = imread(file, 0);
    if (!frame.data) {
	cout<<"Calibration: "<<"file not found: "<<file<<endl;
        return false;
    }
    //cout << "Calibration frame size: "<<
    //    frame.cols<<" "<<frame.rows<<endl;
    initChessboard(isCamera);
    if (!findChessboard(frame)) {
	cout << "Calibration: "<<
            "could not find chessboard pattern"<<endl;
        return false;
    }
    
    return doCalibration(m_camPxPoints,m_realMmPoints,isCamera);
}

void Calibration::initChessboard(bool isCamera)
{
    cv::Size size = m_calibData.chessboardSize;
    float cellSize = m_calibData.chessboardCellSize;

    float centerX = ((float)size.width - 1.f) / 2.f;
    float centerY = ((float)size.height - 1.f) / 2.f;
    int nCells = size.width * size.height;
    m_realMmPoints = std::vector<cv::Point2f>(nCells);
    m_camPxPoints = std::vector<cv::Point2f>(nCells);
    int index = 0;
    for (int i = 0; i < size.height; i++)
    {
        for (int j = 0; j < size.width; j++, index++)
        {
            if (isCamera) {
                m_realMmPoints[index].x =
                    ((float)j-centerX) * cellSize;
                m_realMmPoints[index].y =
                    ((float)i-centerY) * cellSize;
            } else {
                m_realMmPoints[index].x =
                    ((float)j-centerX) * cellSize;
                m_realMmPoints[index].y =
                    -((float)i-centerY) * cellSize;
            }
        }
    }
}

void Calibration::setFrameSize(const cv::Size &size)	{
	m_calibData.frameSize = size;
}

void Calibration::setNearPlane(int nearPlane) {
	m_calibData.nearPlane = nearPlane;
}

void Calibration::setFarPlane(int farPlane) {
	m_calibData.farPlane = farPlane;
}

void Calibration::setChessboardSize(const cv::Size &size) {
	m_calibData.chessboardSize = size;
}

void Calibration::setChessboardCellSize(int size) {
	m_calibData.chessboardCellSize = size;
}

void Calibration::setIntrinsicMatrix(const std::vector<float> &intrinsic) {
	m_intrinsic = cv::Mat(3,3,CV_32FC1);
	for (int i=0; i<3; i++) {
		for (int j=0; j<3; j++) {
			m_intrinsic.at<float>(i,j) = intrinsic[i*3+j];
		}
	}
}

void Calibration::setDistortionMatrix(const std::vector<float> &distortion) {
	m_distortion = cv::Mat(5,1,CV_32FC1);
	for (unsigned int i=0; i<distortion.size(); ++i) {
		m_distortion.at<float>(i) = distortion[i];
	}
}

bool Calibration::findChessboard(const cv::Mat &frame) {
    return findChessboardCorners(frame, m_calibData.chessboardSize, m_camPxPoints,
			CV_CALIB_CB_ADAPTIVE_THRESH + CV_CALIB_CB_FILTER_QUADS
					+ CV_CALIB_CB_FAST_CHECK);
} 

std::vector<float> Calibration::getProjectionMatrix(bool isCamera) const {
    vector<float> proj(16);
    int width = m_calibData.frameSize.width;
    int height = m_calibData.frameSize.height;
    float fx = m_intrinsic.at<float>(0,0);
    float fy = m_intrinsic.at<float>(1,1);
    float cx = m_intrinsic.at<float>(0,2);
    if (!isCamera) {
        cx = m_calibData.frameSize.width - m_intrinsic.at<float>(0,2);
    }
    float cy = m_intrinsic.at<float>(1,2);
    double near = m_calibData.nearPlane;
    double far = m_calibData.farPlane;
    proj[0] = 2.f * fx / (double) width;
    proj[1] = 0;
    proj[2] = 0;
    proj[3] = 0;
    proj[4] = 0;
    proj[5] = 2.0 * fy / (double)height;
    proj[6] = 0;
    proj[7] = 0;
    proj[8] = 2.0 * (cx / (double) width) - 1.0;
    proj[9] = 2.0 * (cy / (double) height) - 1.0;
    proj[10] = -(far + near) / (far - near);
    proj[11] = -1.0;
    proj[12] = 0;
    proj[13] = 0;
    proj[14] = -2.0 * far * near / (far -near);
    proj[15] = 0;
    return proj;
}

std::vector<float> Calibration::getMvMatrix(bool isCamera) const {
    Mat mat = Mat::eye(4,4,CV_32F);
    Mat result = mat.clone();
    Mat roi(mat, Rect(0,0,3,3));
    m_rotation.copyTo(roi);
    mat.at<float>(0,3) = m_translation.at<float>(0,0);
    mat.at<float>(1,3) = m_translation.at<float>(1,0);
    mat.at<float>(2,3) = m_translation.at<float>(2,0);
    mat.at<float>(3,3) = 1.0;

    if (!isCamera && m_calibData.positiveViewDirection) {
        Mat scale = Mat::eye(4,4,CV_32F);
        scale.at<float>(1,1) = -1.f;
        scale.at<float>(2,2) = -1.f;
        result = scale*mat;
    } else {
        mat.copyTo(result);
    }
    vector<float> modelview(16);
    int index=0;
    for (int i=0; i<4; i++) {
        for (int j=0; j<4; j++) {
            modelview[index++] = result.at<float>(j,i);
        }
    }
    return modelview;
}
