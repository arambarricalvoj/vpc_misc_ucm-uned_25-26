/*
Transformada Laplaciana del Gaussiano (LoG) con detección de bordes por zero-crossings relativos
Objetivo: detección robusta de bordes en imágenes digitales

Flujo de trabajo:
 1. Suavizado gaussiano (reduce ruido antes de aplicar la Laplaciana).
 2. Cálculo de la Laplaciana (varios núcleos posibles).
 3. Umbral relativo basado en la mediana absoluta de la respuesta.
 4. Detección de zero-crossings (cambios de signo con magnitud suficiente).
 5. Generación de la imagen binaria de bordes.

Ejecución:
./kim_aggarwal input.png output.png sigma filterType kRel
    - input.png: imagen de entrada en escala de grises.
    - output.png: nombre del archivo de salida (imagen de bordes).
    - sigma: desviación estándar del filtro gaussiano (ej. 1.0).
    - filterType: tipo de núcleo Laplaciano (4, 6, 8).
    - kRel: factor relativo para el umbral (ej. 0.5).
*/

#include "filtroGaussiano.hpp"   // funciones auxiliares: createGaussianKernel, applyConvolution
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <numeric>
#include <string>

using namespace cv;

//-----------------------------------------------------------------------------------------------
// Función principal: detección de bordes por zero-crossings tras LoG
//-----------------------------------------------------------------------------------------------
Mat zeroCrossingLoG(const Mat& imgGray, const int& filterType = 4, double sigma = 1.0, double kRel = 0.5) {
    CV_Assert(imgGray.type() == CV_8U); // comprobación: imagen en escala de grises 8 bits

    // 1) Suavizado gaussiano
    int ksize = std::max(3, int(std::ceil(6*sigma)) | 1); // tamaño ~ 6σ, impar
    Mat blurred;
    Mat gaussKernel = createGaussianKernel(ksize, sigma); // kernel gaussiano personalizado
    blurred = applyConvolution(imgGray, gaussKernel);     // convolución con el kernel

    // 2) Laplaciana (en float para conservar signo y magnitud)
    Mat lap;
    Mat kernel;
    if (filterType == 6) {
        kernel = (Mat_<float>(3,3) <<
               1, -2, 1,
              -2,  4, -2,
               1, -2, 1); // Laplaciano ponderado
    } else if (filterType == 8) {
        kernel = (Mat_<float>(3,3) <<
              -1, -1, -1,
              -1,  8, -1,
              -1, -1, -1); // Laplaciano 8-conexo
    } else {
        kernel = (Mat_<float>(3,3) <<
               0, -1, 0,
              -1,  4, -1,
               0, -1, 0); // Laplaciano 4-conexo
    }
    filter2D(blurred, lap, CV_32F, kernel);

    // 3) Umbral relativo basado en percentil (mediana absoluta ~ P50)
    Mat absLap = abs(lap);
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
                    float diff = std::abs(c) + std::abs(v);
                    if (diff > maxOppDiff) maxOppDiff = diff;
                }
            }
            // criterio relativo: cambio de signo + magnitud suficiente
            if (zeroCross && maxOppDiff >= thr) {
                edges.at<uint8_t>(y, x) = 255;
            }
        }
    }

    return edges;
}

//-----------------------------------------------------------------------------------------------
// Programa principal
//-----------------------------------------------------------------------------------------------
int main(int argc, char** argv) {
    if (argc < 6) {
        std::cerr << "Uso: " << argv[0] << " <imagen_entrada> <imagen_salida> <sigma> <filtro laplaciano> <kRel>\n";
        return -1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    double sigma = atof(argv[3]);
    int lap_filter  = atoi(argv[4]);
    double kRel  = atof(argv[5]);

    // Cargar imagen en escala de grises
    cv::Mat img = cv::imread(inputFile, cv::IMREAD_GRAYSCALE);
    if (img.empty()) {
        std::cerr << "No se pudo abrir la imagen: " << inputFile << std::endl;
        return -1;
    }

    // Aplicar detección de bordes LoG con zero-crossings relativos
    cv::Mat edges = zeroCrossingLoG(img, lap_filter, sigma, kRel);

    // Guardar resultado
    cv::imwrite(outputFile, edges);
    std::cout << "Imagen de bordes guardada en: " << outputFile << std::endl;

    return 0;
}
