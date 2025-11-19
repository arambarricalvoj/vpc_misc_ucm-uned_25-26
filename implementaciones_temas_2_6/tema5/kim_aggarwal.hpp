#ifndef KIM_AGGARWAL_HPP
#define KIM_AGGARWAL_HPP

#include <opencv2/opencv.hpp>
using namespace cv;

Mat zeroCrossingLoG(const Mat& imgGray, const int& filterType, double sigma, double kRel);

#endif // KIM_AGGARWAL_HPP
