#include "filtroGaussiano.hpp"
#include <opencv2/opencv.hpp>
using namespace cv;

// Detecta bordes por zero-crossings relativos tras LoG
Mat zeroCrossingLoG(const Mat& imgGray, const int& filterType = 4, double sigma = 1.0, double kRel = 0.5) {
    CV_Assert(imgGray.type() == CV_8U);

    // 1) Suavizado gaussiano
    int ksize = std::max(3, int(std::ceil(6*sigma)) | 1); // tamaño ~ 6σ, impar
    Mat blurred;

    //GaussianBlur(imgGray, blurred, Size(ksize, ksize), sigma, sigma);
    cv::Mat gaussKernel = createGaussianKernel(ksize, sigma);
    blurred = applyConvolution(imgGray, gaussKernel);


    // 2) Laplaciana (en float para signo y magnitud)
    // Mat lap;
    // Laplacian(blurred, lap, CV_32F, 3); // kernel 3x3
    Mat lap;
    Mat kernel;
    if (filterType == 6)
    {
        kernel = (Mat_<float>(3,3) <<
               1, -2, 1,
               -2,  4, -2,
               1, -2, 1); // Laplaciano ponderado
    } 
    else if (filterType == 8)
    {
        kernel = (Mat_<float>(3,3) <<
               -1, -1, -1,
               -1,  8, -1,
               -1, -1, -1); // Laplaciano 8-conexo
    }
    else
    {
        kernel = (Mat_<float>(3,3) <<
               0, -1, 0,
               -1,  4, -1,
               0, -1, 0);
    }
    
    filter2D(blurred, lap, CV_32F, kernel);



    // 3) Umbral relativo basado en percentil (mediana absoluta ~ P50)
    //    Estima robusta del nivel de respuesta laplaciana
    Mat absLap;
    absLap = abs(lap);

    // Calcular percentil 50 (mediana) de |lap|
    // Convertimos a vector
    std::vector<float> vals;
    vals.reserve(absLap.rows * absLap.cols);
    for (int y = 0; y < absLap.rows; ++y) {
        const float* row = absLap.ptr<float>(y);
        for (int x = 0; x < absLap.cols; ++x) vals.push_back(row[x]);
    }
    std::nth_element(vals.begin(), vals.begin() + vals.size()/2, vals.end());
    float medAbs = vals[vals.size()/2];

    float thr = kRel * medAbs; // umbral relativo

    // 4) Zero-crossing relativo
    Mat edges = Mat::zeros(imgGray.size(), CV_8U);
    for (int y = 1; y < lap.rows - 1; ++y) {
        const float* r0 = lap.ptr<float>(y-1);
        const float* r1 = lap.ptr<float>(y);
        const float* r2 = lap.ptr<float>(y+1);
        for (int x = 1; x < lap.cols - 1; ++x) {
            float c = r1[x];
            // vecinos 8-conexos
            float nb[8] = { r0[x-1], r0[x], r0[x+1],
                            r1[x-1],           r1[x+1],
                            r2[x-1], r2[x], r2[x+1] };

            bool zeroCross = false;
            float maxOppDiff = 0.f;

            for (int k = 0; k < 8; ++k) {
                float v = nb[k];
                if ((c > 0 && v < 0) || (c < 0 && v > 0)) {
                    zeroCross = true;
                    // diferencia de magnitudes entre signos opuestos
                    float diff = std::abs(c) + std::abs(v);
                    if (diff > maxOppDiff) maxOppDiff = diff;
                }
            }

            // criterio relativo (Kim & Aggarwal): cambio de signo + magnitud suficiente
            if (zeroCross && maxOppDiff >= thr) {
                edges.at<uint8_t>(y, x) = 255;
            }
        }
    }

    return edges;
}

//cv::Mat edges = zeroCrossingLoG(img, lap_filter, sigma, kRel);


