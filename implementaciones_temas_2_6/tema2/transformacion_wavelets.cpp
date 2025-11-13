/* Transformada Discreta de Wavelets (Haar DWT)
Objetivo: compresión de imágenes digitales mediante descomposición multiresolución

Flujo de trabajo:
 - 1. preparación de datos (lectura en escala de grises y normalización),
 - 2. aplicación de la transformada Haar 2D (filtrado paso bajo/alto en filas y columnas),
 - 3. obtención de coeficientes LL, LH, HL, HH,
 - 4. umbralización de coeficientes de alta frecuencia (LH, HL, HH),
 - 5. reconstrucción mediante la transformada inversa Haar (IDWT),
 - 6. guardado de la imagen comprimida y de las sub-bandas intermedias.

Ejecución:
./wavelet lena_input.png lena_output.png 0.02
    - input_image: nombre de la imagen a comprimir (tiene que estar en la carpeta input)
    - output_image: nombre del archivo de salida (se almacena en la carpeta output)
    - threshold: umbral de compresión aplicado a los coeficientes de alta frecuencia.

Salida (en la carpeta output):
    - Imagen comprimida reconstruida en la carpeta output.
    - Imagen original en escala de grises para comparación.
    - Sub-bandas LL, LH, HL, HH y matriz completa de coeficientes guardadas en output/.
    - Matriz de coeficientes tras umbralización (coeffs_threshold.png).
*/

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <filesystem>
#include <cmath>

/////////////////////////////////////////////////////////////////////////////////////////////////
// Transformada Haar 1D directa (in-place)
/////////////////////////////////////////////////////////////////////////////////////////////////
static void haar1D(double* data, int n) {
    double* temp = new double[n];
    for (int i = 0; i < n / 2; ++i) {
        double a = data[2 * i];
        double b = data[2 * i + 1];
        temp[i]           = (a + b) / std::sqrt(2.0);        // coeficiente de aproximación
        temp[i + n / 2]   = (a - b) / std::sqrt(2.0);        // coeficiente de detalle
    }
    std::memcpy(data, temp, n * sizeof(double));
    delete[] temp;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Transformada Haar 1D inversa (in-place)
/////////////////////////////////////////////////////////////////////////////////////////////////
static void ihaar1D(double* data, int n) {
    double* temp = new double[n];
    for (int i = 0; i < n / 2; ++i) {
        double avg  = data[i];
        double diff = data[i + n / 2];
        temp[2 * i]     = (avg + diff) / std::sqrt(2.0);
        temp[2 * i + 1] = (avg - diff) / std::sqrt(2.0);
    }
    std::memcpy(data, temp, n * sizeof(double));
    delete[] temp;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Transformada Haar 2D directa (in-place)
/////////////////////////////////////////////////////////////////////////////////////////////////
static void haar2D(cv::Mat& img64) {
    int rows = img64.rows;
    int cols = img64.cols;

    // Filtrado por filas
    for (int r = 0; r < rows; ++r) {
        double* row = img64.ptr<double>(r);
        haar1D(row, cols);
    }

    // Filtrado por columnas
    double* colBuff = new double[rows];
    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows; ++r) colBuff[r] = img64.at<double>(r, c);
        haar1D(colBuff, rows);
        for (int r = 0; r < rows; ++r) img64.at<double>(r, c) = colBuff[r];
    }
    delete[] colBuff;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Transformada Haar 2D inversa (in-place)
/////////////////////////////////////////////////////////////////////////////////////////////////
static void ihaar2D(cv::Mat& img64) {
    int rows = img64.rows;
    int cols = img64.cols;

    double* colBuff = new double[rows];
    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows; ++r) colBuff[r] = img64.at<double>(r, c);
        ihaar1D(colBuff, rows);
        for (int r = 0; r < rows; ++r) img64.at<double>(r, c) = colBuff[r];
    }
    delete[] colBuff;

    for (int r = 0; r < rows; ++r) {
        double* row = img64.ptr<double>(r);
        ihaar1D(row, cols);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Umbralización dura (in-place)
/////////////////////////////////////////////////////////////////////////////////////////////////
static void hardThreshold(cv::Mat& coeffs, double thr) {
    for (int r = 0; r < coeffs.rows; ++r) {
        double* row = coeffs.ptr<double>(r);
        for (int c = 0; c < coeffs.cols; ++c) {
            if (std::abs(row[c]) < thr) row[c] = 0.0;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Clipping a [0,1] (in-place)
/////////////////////////////////////////////////////////////////////////////////////////////////
static void clip01(cv::Mat& img64) {
    for (int r = 0; r < img64.rows; ++r) {
        double* row = img64.ptr<double>(r);
        for (int c = 0; c < img64.cols; ++c) {
            row[c] = std::min(1.0, std::max(0.0, row[c]));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Guardar sub-bandas normalizadas
/////////////////////////////////////////////////////////////////////////////////////////////////
static void saveBand(const cv::Mat& band, const std::string& path) {
    cv::Mat normBand;
    cv::normalize(band, normBand, 0, 255, cv::NORM_MINMAX);
    normBand.convertTo(normBand, CV_8U);
    cv::imwrite(path, normBand);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Programa principal
/////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <input_image> <output_image> [threshold]\n";
        return 1;
    }

    std::string input_path  = argv[1];
    std::string output_path = argv[2];
    double threshold        = (argc >= 4) ? std::stod(argv[3]) : 0.02;

    // Cargar imagen en escala de grises
    cv::Mat img8 = cv::imread("input/" + input_path, cv::IMREAD_GRAYSCALE);
    if (img8.empty()) {
        std::cerr << "No se pudo cargar la imagen: " << input_path << "\n";
        return 1;
    }

    // Convertir a double y normalizar a [0,1]
    cv::Mat img64;
    img8.convertTo(img64, CV_64F, 1.0 / 255.0);

    if ((img64.rows % 2) != 0 || (img64.cols % 2) != 0) {
        std::cerr << "Dimensiones deben ser pares para Haar 1 nivel.\n";
        return 1;
    }

    // DWT
    haar2D(img64);

    // Construir sufijo con threshold
    std::string thr_str = "_" + std::to_string(threshold);

    // Guardar coeficientes y sub-bandas con threshold en el nombre
    saveBand(img64, "output/coeffs_full" + thr_str + ".png");
    int halfRows = img64.rows / 2;
    int halfCols = img64.cols / 2;
    saveBand(img64(cv::Rect(0,         0,         halfCols, halfRows)), "output/coeffs_LL" + thr_str + ".png");
    saveBand(img64(cv::Rect(halfCols,  0,         halfCols, halfRows)), "output/coeffs_LH" + thr_str + ".png");
    saveBand(img64(cv::Rect(0,         halfRows,  halfCols, halfRows)), "output/coeffs_HL" + thr_str + ".png");
    saveBand(img64(cv::Rect(halfCols,  halfRows,  halfCols, halfRows)), "output/coeffs_HH" + thr_str + ".png");

    // Umbralización
    hardThreshold(img64, threshold);
    saveBand(img64, "output/coeffs_threshold" + thr_str + ".png");

    // Reconstrucción
    cv::Mat recon = img64.clone();
    ihaar2D(recon);
    clip01(recon);

    cv::Mat out8;
    recon.convertTo(out8, CV_8U, 255.0);

    // Guardar reconstrucción y original gris con threshold en el nombre
    std::filesystem::path outp("output/" + output_path);
    std::filesystem::path outdir = outp.parent_path();
    std::filesystem::path stem   = outp.stem();
    std::filesystem::path ext    = outp.extension();

    std::filesystem::path out_with_thr  = outdir / (stem.string() + thr_str + ext.string());
    std::filesystem::path orig_out      = outdir / (stem.string() + "_original_gris" + thr_str + ext.string());

    cv::imwrite(out_with_thr.string(), out8);
    cv::imwrite(orig_out.string(), img8);

    // Mensajes informativos
    std::cout << "Wavelet (Haar) compresión aplicada.\n";
    std::cout << "Umbral = " << threshold << "\n";
    std::cout << "Guardado reconstrucción: " << out_with_thr << "\n";
    std::cout << "Guardado original gris:  " << orig_out << "\n";
    std::cout << "Guardados coeficientes en carpeta output/.\n";

    return 0;
}
