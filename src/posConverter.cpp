#include "posConverter.h"

#include <string>
#include <iostream>

using namespace std;

PosConverter::PosConverter() :
    m_intrinsic(cv::Mat::eye(3, 3, CV_32FC1)),
    m_intrinsicInv(cv::Mat::eye(3, 3, CV_32FC1)),
    m_rotation(cv::Mat::eye(3,3,CV_32FC1)),
    m_rotationInv(cv::Mat::eye(3,3,CV_32FC1)) {
}

PosConverter::~PosConverter() {
}

void PosConverter::setIntrinsic(const cv::Mat &mat) {
	mat.copyTo(m_intrinsic);
        m_intrinsicInv = m_intrinsic.inv();
}

void PosConverter::setRotation(const cv::Mat &mat) {
	mat.copyTo(m_rotation);
        m_rotationInv = m_rotation.inv();
}

void PosConverter::setTranslation(const cv::Mat &mat) {
	mat.copyTo(m_translation);
}


cv::Point2f PosConverter::pixel2mm(const cv::Point2f &p,
		float height) const {
	return pixel2mm(p.x, p.y, height);
}

cv::Point2f PosConverter::pixel2mm(float px_x, float px_y,
		float height) const {
	cv::Mat rotationMat, transVector, invA, invRot;

	rotationMat = m_rotation;
	invRot = m_rotationInv;
	transVector = m_translation;
	invA = m_intrinsicInv;

	cv::Mat point = cv::Mat(3, 1, CV_32FC1);
	point.at<float> (0, 0) = px_x;
	point.at<float> (1, 0) = px_y;
	point.at<float> (2, 0) = 1;

	point = invA * point;

	double Z = height;
	double a = rotationMat.at<float> (0, 0) / rotationMat.at<float> (1, 0);
	double b = rotationMat.at<float> (0, 0) / rotationMat.at<float> (2, 0);
	double c = rotationMat.at<float> (0, 1) - a * rotationMat.at<float> (1, 1);
	double d = rotationMat.at<float> (0, 2) - a * rotationMat.at<float> (1, 2);
	double e = rotationMat.at<float> (0, 1) - b * rotationMat.at<float> (2, 1);
	double f = rotationMat.at<float> (0, 2) - b * rotationMat.at<float> (2, 2);
	double g = (d - f * c / e) * Z + (1 - c / e) * transVector.at<float> (0, 0)
			- a * transVector.at<float> (1, 0) + (b * c / e) * transVector.at<
			float> (2, 0);
	double h = (1 - c / e) * point.at<float> (0, 0) - a
			* point.at<float> (1, 0) + b * c / e;

	double zp = g / h;
	point.at<float> (2, 0) = zp;
	point.at<float> (0, 0) = point.at<float> (0, 0) * zp;
	point.at<float> (1, 0) *= zp;

	point = point - transVector;
	point = invRot * point;

        cv::Point2f result;
        result.x = point.at<float> (0, 0);
        result.y = point.at<float> (1, 0);
        return result;
}

cv::Point2f PosConverter::mm2pixel(const cv::Point2f &p,
		 float height) const {
	return mm2pixel(p.x, p.y, height);
}

cv::Point2f PosConverter::mm2pixel(float x, float y,
		float height) const {
	cv::Mat rotationMat, transVector, A;
	rotationMat = m_rotation;
	transVector = m_translation;
	A = m_intrinsic;

	cv::Mat point = cv::Mat(3, 1, CV_32FC1);
	point.at<float> (0, 0) = x;
	point.at<float> (1, 0) = y;
	point.at<float> (2, 0) = height;

	// (R|T)*M = m'
	point = rotationMat * point;
	point = point + transVector;
	// m = m'/m'_z
	double scale = point.at<float> (2, 0);
	point *= 1.0 / scale;

	// m = Am
	point = A * point;

        cv::Point2f result;
        result.x = point.at<float> (0, 0);
        result.y = point.at<float> (1, 0);
        return result;
}

void PosConverter::printMatrix(const cv::Mat &mat, string name) const {
	cout << "PosConverter: " << name << endl;
	cout.precision(3);
	for (int i = 0; i < mat.rows; i++) {
		for (int j = 0; j < mat.cols; j++) {
			cout << mat.at<float> (i, j) << " ";
		}
		cout << endl;
	}
}

