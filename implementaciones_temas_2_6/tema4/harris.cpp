/*
Detección de puntos de interés con Harris (versión directa OpenCV)
Sin binarización, sin supresión de no máximos y sin refinado subpíxel

Ejecución:
./harris_simple input.png output_prefix blockSize ksize k [thresh_percent]

Ejemplo:
./harris lena.png results 2 3 0.04 1.0
 - blockSize = 2
 - ksize = 3
 - k = 0.04
 - thresh_percent = 1.0   (umbral relativo: 1.0% de R_max). Opcional; por defecto 1.0
*/

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
    if (argc < 6) {
        cerr << "Uso:\n"
             << "  " << argv[0] << " <imagen_entrada> <output_prefix> <blockSize> <ksize> <k> [thresh_percent]\n";
        return -1;
    }

    string inputFile = argv[1];
    string outPrefix = argv[2];
    int blockSize = atoi(argv[3]);
    int ksize = atoi(argv[4]);
    double k = atof(argv[5]);

    double thresh_percent = 1.0;
    if (argc >= 7) thresh_percent = atof(argv[6]);

    Mat src = imread(inputFile, IMREAD_GRAYSCALE);
    if (src.empty()) {
        cerr << "No se pudo abrir la imagen: " << inputFile << endl;
        return -1;
    }

    Mat src_float;
    src.convertTo(src_float, CV_32F, 1.0 / 255.0);

    // Calcular respuesta Harris (dst es CV_32F)
    Mat dst;
    cornerHarris(src_float, dst, blockSize, ksize, k, BORDER_DEFAULT);

    // Normalizar respuesta para visualización (CV_32F -> [0,255]) y guardar
    Mat dst_norm, dst_norm_scaled;
    normalize(dst, dst_norm, 0, 255, NORM_MINMAX, CV_32F);
    convertScaleAbs(dst_norm, dst_norm_scaled);
    imwrite(outPrefix + "_harris_response.png", dst_norm_scaled);

    // Umbral relativo para marcar puntos sobre la imagen (sin binarización persistente)
    double minVal, maxVal;
    minMaxLoc(dst, &minVal, &maxVal);
    double thresh = (thresh_percent / 100.0) * maxVal;

    // Superponer puntos donde R > thresh (sin supresión de no máximos)
    Mat vis;
    cvtColor(src, vis, COLOR_GRAY2BGR);
    int count = 0;
    for (int y = 0; y < dst.rows; ++y) {
        for (int x = 0; x < dst.cols; ++x) {
            if (dst.at<float>(y, x) > thresh) {
                // dibujar un punto pequeño (se pueden ver agrupaciones sin NMS)
                circle(vis, Point(x, y), 1, Scalar(0, 0, 255), FILLED, LINE_AA);
                ++count;
            }
        }
    }

    imwrite(outPrefix + "_harris_corners.png", vis);

    cout << "Respuesta Harris guardada: " << outPrefix << "_harris_response.png\n";
    cout << "Visualización con puntos (sin NMS ni refinado): " << outPrefix << "_harris_corners.png\n";
    cout << "Número de puntos marcados (R > thresh): " << count << "\n";

    return 0;
}
