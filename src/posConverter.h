#ifndef _AR_COORDINATEMAPPER_H
#define _AR_COORDINATEMAPPER_H

#include <memory>
#include <opencv2/opencv.hpp>

class PosConverter {
public:
	PosConverter();
	virtual ~PosConverter();

	// pixel to mm coordinates
        cv::Point2f pixel2mm(const cv::Point2f &p, float height = 0) const;
        cv::Point2f pixel2mm(float px_x, float px_y,
			float height = 0) const;

	// mm to pixel coordinates
        cv::Point2f mm2pixel(const cv::Point2f &p, float height = 0) const;
        cv::Point2f mm2pixel(float x, float y,
			float height = 0) const;

	void setIntrinsic(const cv::Mat &mat);
        void setRotation(const cv::Mat &mat);
        void setTranslation(const cv::Mat &mat);
	void printMatrix(const cv::Mat &mat, std::string name) const;

private:
	PosConverter(const PosConverter &);

private:
        cv::Mat m_intrinsic, m_intrinsicInv;
        cv::Mat m_rotation, m_rotationInv;
        cv::Mat m_translation;
};

#endif //_AR_COORDINATEMAPPER_H

