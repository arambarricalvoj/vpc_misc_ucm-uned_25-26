#ifndef FILTRO_GAUSSIANO_HPP
#define FILTRO_GAUSSIANO_HPP

#include <opencv2/opencv.hpp>

// Construye un kernel gaussiano normalizado de tamaño ksize x ksize
cv::Mat createGaussianKernel(int ksize, double sigma);

// Aplica convolución manual de un kernel sobre una imagen en escala de grises
cv::Mat applyConvolution(const cv::Mat& img, const cv::Mat& kernel);

#endif // FILTRO_GAUSSIANO_HPP
