/*
Detección de bordes con Sobel
Objetivo: calcular gradiente en X e Y y obtener magnitud de bordes

Este código incluye dos versiones:
 1. Implementación manual con filter2D y kernels Sobel definidos a mano.
 2. Una implementación directa mediante la función cv::Sobel, que permite variar el tamaño del kernel y el orden de la derivada.

Selección de versión:
 - Se pasa como argumento "manual" o "opencv" en la línea de comandos.

Ejecución:
Versión manual:
./sobel input.png output.png manual bin_threshold

Versión opencv:
./sobel input.png output.png opencv bin_threshold ksize
*/

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
using namespace cv;

int main(int argc, char** argv) {
    if (argc < 5) {
        std::cerr << "Uso:\n"
                  << "  " << argv[0] << " <imagen_entrada> <imagen_salida> manual <bin_threshold>\n"
                  << "  " << argv[0] << " <imagen_entrada> <imagen_salida> opencv <bin_threshold> <ksize>\n";
        return -1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    std::string version = argv[3];
    int threshVal = atoi(argv[4]);

    int ksize = 3; // valor por defecto
    if (version == "opencv") {
        if (argc < 6) {
            std::cerr << "Falta el parámetro ksize para la versión opencv.\n";
            return -1;
        }
        ksize = atoi(argv[5]);
    }

    // 1) Cargar imagen en escala de grises
    Mat img = imread(inputFile, IMREAD_GRAYSCALE);
    if (img.empty()) {
        std::cerr << "No se pudo abrir la imagen: " << inputFile << std::endl;
        return -1;
    }

    Mat grad_x, grad_y;

    // ============================================================
    // VERSIÓN 1: MANUAL con filter2D
    // ============================================================
    if (version == "manual") {
        Mat sobelX = (Mat_<float>(3,3) <<
            -1, 0, 1,
            -2, 0, 2,
            -1, 0, 1);

        Mat sobelY = (Mat_<float>(3,3) <<
            -1, -2, -1,
             0,  0,  0,
             1,  2,  1);

        filter2D(img, grad_x, CV_32F, sobelX);
        filter2D(img, grad_y, CV_32F, sobelY);
    }

    // ============================================================
    // VERSIÓN 2: DIRECTA con cv::Sobel
    // ============================================================
    else if (version == "opencv") {
        Sobel(img, grad_x, CV_32F, 1, 0, ksize); // derivada en X
        Sobel(img, grad_y, CV_32F, 0, 1, ksize); // derivada en Y
    } else {
        std::cerr << "Versión no reconocida. Use 'manual' o 'opencv'.\n";
        return -1;
    }

    // ============================================================
    // Exportar derivadas intermedias
    // ============================================================
    Mat grad_x_norm, grad_y_norm;
    normalize(grad_x, grad_x_norm, 0, 255, NORM_MINMAX);
    normalize(grad_y, grad_y_norm, 0, 255, NORM_MINMAX);
    grad_x_norm.convertTo(grad_x_norm, CV_8U);
    grad_y_norm.convertTo(grad_y_norm, CV_8U);

    imwrite("x_" + outputFile, grad_x_norm);
    imwrite("y_" + outputFile, grad_y_norm);

    // ============================================================
    // Magnitud del gradiente
    // ============================================================
    Mat magnitude = abs(grad_x) + abs(grad_y); // aproximación rápida
    Mat mag8;
    normalize(magnitude, magnitude, 0, 255, NORM_MINMAX);
    magnitude.convertTo(mag8, CV_8U);

    // ============================================================
    // Binarización de bordes
    // ============================================================
    Mat edges;
    cv::threshold(mag8, edges, threshVal, 255, THRESH_BINARY);

    // Guardar resultados
    imwrite(outputFile, mag8);            // magnitud continua
    imwrite("bin_" + outputFile, edges);  // bordes binarios

    std::cout << "Guardado: " << outputFile << " (magnitud)\n";
    std::cout << "Guardado: bin_" << outputFile << " (bordes binarios)\n";
    std::cout << "Derivadas intermedias guardadas: x_" << outputFile << ", y_" << outputFile << "\n";

    return 0;
}
