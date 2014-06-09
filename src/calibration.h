#ifndef _webserver_app_calibration_h_
#define _webserver_app_calibration_h_

#include <opencv2/opencv.hpp>
#include <vector>

class Calibration
{
public:
    Calibration();
    ~Calibration();

    void setFrameSize(const cv::Size &frameSize);	
    void setNearPlane(int nearPlane);
    void setFarPlane(int farPlane);
    void setChessboardSize(const cv::Size &size);
    void setChessboardCellSize(int size);
    void setIntrinsicMatrix(const std::vector<float> &intrinsic);
    void setDistortionMatrix(const std::vector<float> &distortion);

    bool calibrate(const std::string &file,bool isCamera);
    bool calibrateFromPoints(std::vector<float> imagePoints, std::vector<float> objectPoints, bool isCamera);

    std::vector<float> getProjectionMatrix(bool isCamera) const;
    std::vector<float> getMvMatrix(bool isCamera) const;

private:
    void initChessboard(bool isCamera);
    bool findChessboard(const cv::Mat &frame);

    bool doCalibration(const std::vector<cv::Point2f> &camPoints, const std::vector<cv::Point2f> &realPoints, bool isCamera);

    void testConversion(std::vector<cv::Point2f> &mmPoints,
        std::vector<cv::Point2f> &pxPoints); 

private:
    struct CalibData_t
    {
        float nearPlane, farPlane;
        cv::Size chessboardSize;
        float chessboardCellSize;
        cv::Size frameSize;
        bool positiveViewDirection;

        CalibData_t(): nearPlane(0.f),
            farPlane(1000.f),
            chessboardSize(12, 14),
            chessboardCellSize(10),
            frameSize(640, 480),
            positiveViewDirection(true) {}
    };
	
    CalibData_t m_calibData;

    std::vector<cv::Point2f> m_realMmPoints;
    std::vector<cv::Point2f> m_camPxPoints;

    cv::Mat m_intrinsic, m_distortion;
    cv::Mat m_rotation, m_translation;
};

#endif //  _webserver_app_calibration_h_
