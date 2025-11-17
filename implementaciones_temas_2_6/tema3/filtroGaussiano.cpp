/*
Filtro Gaussiano con Convolución Manual
Objetivo: aplicar un suavizado gaussiano a una imagen digital, construyendo
          explícitamente el kernel gaussiano y realizando la convolución manual
          sobre cada canal de color.

Contexto:
 - OpenCV ofrece la función cv::GaussianBlur para aplicar directamente el filtro.
 - En este ejemplo se implementa el cálculo del kernel y la convolución para
   comprender mejor el funcionamiento interno del filtrado gaussiano.

Flujo de trabajo:
 1. Leer parámetros desde línea de comandos:
    <imagen_entrada> <imagen_salida> <tam_kernel> <sigma>
    - imagen_entrada: ruta de la imagen original.
    - imagen_salida: ruta del archivo resultante.
    - tam_kernel: tamaño del kernel (ej. 3, 5, 7, 9).
    - sigma: desviación estándar de la gaussiana.
 2. Construir el kernel gaussiano normalizado.
 3. Separar la imagen en canales BGR.
 4. Aplicar convolución manual con el kernel a cada canal.
 5. Recomponer la imagen suavizada y guardarla en disco.

Ejemplo de ejecución:
./filtroGaussiano lena.png lena_out.png 7 1.5

Salida:
 - Imagen suavizada guardada en <imagen_salida>.

Notas:
 - La convolución manual es más lenta que cv::GaussianBlur, pero útil para fines
   didácticos.
 - Se normaliza el kernel para que la suma de sus coeficientes sea 1.
 - Se procesa cada canal por separado y luego se combinan.
*/

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>

////////////////////////////////////////////////////////////////////////////////
// Construcción del kernel gaussiano
////////////////////////////////////////////////////////////////////////////////
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

    // Normalizar para que la suma de coeficientes sea 1
    kernel /= sum;
    return kernel;
}

////////////////////////////////////////////////////////////////////////////////
// Convolución manual sobre una imagen monocanal
// Nota: en OpenCV se puede usar directamente cv::filter2D(img, blurred, -1, kernel)
////////////////////////////////////////////////////////////////////////////////
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

    // Convertir de vuelta a 8 bits para visualización
    cv::Mat result8U;
    result.convertTo(result8U, CV_8U);
    return result8U;
}

////////////////////////////////////////////////////////////////////////////////
// PROGRAMA PRINCIPAL
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
    if (argc < 5) {
        std::cout << "Uso: " << argv[0] 
                  << " <imagen_entrada> <imagen_salida> <tam_kernel> <sigma>\n";
        return -1;
    }

    // Parámetros desde línea de comandos
    std::string inputPath  = argv[1];
    std::string outputPath = argv[2];
    int ksize              = std::stoi(argv[3]);
    double sigma           = std::stod(argv[4]);

    // Cargar imagen en color
    cv::Mat img = cv::imread(inputPath, cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cerr << "Error: no se pudo cargar la imagen.\n";
        return -1;
    }

    // Crear kernel gaussiano
    cv::Mat kernel = createGaussianKernel(ksize, sigma);

    // Separar canales BGR
    std::vector<cv::Mat> canales;
    cv::split(img, canales);

    // Aplicar convolución manual a cada canal
    for (int c = 0; c < 3; c++) {
        canales[c] = applyConvolution(canales[c], kernel);
    }

    // Recomponer imagen suavizada
    cv::Mat imgBlurred;
    cv::merge(canales, imgBlurred);

    // Guardar resultado
    cv::imwrite(outputPath, imgBlurred);
    std::cout << "Imagen suavizada guardada en: " << outputPath << "\n";

    return 0;
}
