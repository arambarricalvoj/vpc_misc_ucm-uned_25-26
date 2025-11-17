#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>

// Construir kernel gaussiano
cv::Mat createGaussianKernel(int ksize, double sigma) {
    cv::Mat kernel(ksize, ksize, CV_64F);
    int half = ksize / 2;
    double sum = 0.0;

    for (int i = -half; i <= half; i++) {
        for (int j = -half; j <= half; j++) {
            double value = std::exp(-(i*i + j*j) / (2 * sigma * sigma));
            value /= (2 * M_PI * sigma * sigma);
            kernel.at<double>(i + half, j + half) = value;
            sum += value;
        }
    }

    // Normalizar
    kernel /= sum;
    return kernel;
}

// Convolución manual
// En siguiente temas las convoluciones se aplican con 
// cv::Mat blurred;
// cv::filter2D(img, blurred, -1, kernel);
cv::Mat applyConvolution(const cv::Mat& img, const cv::Mat& kernel) {
    int ksize = kernel.rows;
    int half = ksize / 2;
    cv::Mat result = cv::Mat::zeros(img.size(), CV_64F);

    for (int y = half; y < img.rows - half; y++) {
        for (int x = half; x < img.cols - half; x++) {
            double sum = 0.0;
            for (int ky = -half; ky <= half; ky++) {
                for (int kx = -half; kx <= half; kx++) {
                    double pixel = img.at<uchar>(y - ky, x - kx);
                    double weight = kernel.at<double>(ky + half, kx + half);
                    sum += pixel * weight;
                }
            }
            result.at<double>(y, x) = sum;
        }
    }

    // Convertir de vuelta a 8 bits
    cv::Mat result8U;
    result.convertTo(result8U, CV_8U);
    return result8U;
}

//cv::Mat kernel = createGaussianKernel(ksize, sigma);
//applyConvolution(img, kernel);

