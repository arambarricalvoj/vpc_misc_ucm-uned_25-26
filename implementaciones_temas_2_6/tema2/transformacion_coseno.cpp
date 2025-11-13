/* Transformada Discreta del Coseno (DCT)
Objetivo: compresión de imágenes digitales en escala de grises

Flujo de trabajo:
 - 1. Preparación de datos (lectura y normalización),
 - 2. Aplicación de la DCT 2D,
 - 3. Umbralización de coeficientes (hard threshold),
 - 4. Reconstrucción mediante la IDCT,
 - 5. Guardado de resultados.

Ejecución:
./dct input_image.png output_image.png 0.02
    - input_image: nombre de la imagen a comprimir (carpeta input/)
    - output_image: nombre del archivo de salida (carpeta output/)
    - threshold: umbral de compresión (por defecto 0.02)
*/

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <filesystem>
#include <cmath>

/////////////////////////////////////////////////////////////////////////////////////////////////
// Umbralización dura de coeficientes DCT (in-place)
// Pone a cero los coeficientes cuya magnitud es menor que 'thr'.
/////////////////////////////////////////////////////////////////////////////////////////////////
static void hardThreshold(cv::Mat& coeffs, double thr) {
    for (int r = 0; r < coeffs.rows; ++r) {
        double* row = coeffs.ptr<double>(r);
        for (int c = 0; c < coeffs.cols; ++c) {
            if (std::fabs(row[c]) < thr) row[c] = 0.0f;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Programa principal
/////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
    // Lectura de parámetros
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <input_image> <output_image> [threshold]\n";
        std::cerr << "Ejemplo: " << argv[0] << " input/lena.png output/lena_dct.png 0.02\n";
        return 1;
    }

    std::string input_path  = argv[1];
    std::string output_path = argv[2];
    float threshold         = (argc >= 4) ? std::stof(argv[3]) : 0.02f;

    // Cargar imagen en escala de grises
    cv::Mat img8 = cv::imread("input/" + input_path, cv::IMREAD_GRAYSCALE);
    if (img8.empty()) {
        std::cerr << "No se pudo cargar la imagen: " << input_path << "\n";
        return 1;
    }

    // Convertir a float y normalizar en [0,1]
    cv::Mat img64;
    img8.convertTo(img64, CV_64F, 1.0 / 255.0);

    // Aplicar DCT
    cv::Mat coeffs;
    cv::dct(img64, coeffs);

    // Guardar coeficientes normalizados para visualización
    cv::Mat coeffsVis;
    cv::normalize(coeffs, coeffsVis, 0, 255, cv::NORM_MINMAX);
    coeffsVis.convertTo(coeffsVis, CV_8U);
    cv::imwrite("output/coeffs_dct.png", coeffsVis);

    // Umbralización dura (in-place)
    hardThreshold(coeffs, threshold);

    // Guardar coeficientes tras umbralización
    cv::Mat coeffsThrVis;
    cv::normalize(coeffs, coeffsThrVis, 0, 255, cv::NORM_MINMAX);
    coeffsThrVis.convertTo(coeffsThrVis, CV_8U);
    cv::imwrite("output/coeffs_dct_threshold.png", coeffsThrVis);

    // Reconstrucción con IDCT
    cv::Mat recon;
    cv::idct(coeffs, recon);

    // Clipping y conversión a 8 bits
    cv::Mat out8;
    cv::min(recon, 1.0f, recon);
    cv::max(recon, 0.0f, recon);
    recon.convertTo(out8, CV_8U, 255.0);

    /*// Normalización proporcional al rango actual
    double minVal, maxVal;
    cv::minMaxLoc(recon, &minVal, &maxVal);

    // Escalar al rango [0,1]
    cv::Mat reconNorm;
    reconNorm = (recon - minVal) / (maxVal - minVal);

    // Convertir a 8 bits
    cv::Mat out8;
    reconNorm.convertTo(out8, CV_8U, 255.0);*/

    // Guardar reconstrucción y original
    std::filesystem::path outp("output/" + output_path);
    std::filesystem::path outdir = outp.parent_path();
    std::filesystem::path stem   = outp.stem();
    std::filesystem::path ext    = outp.extension();

    std::string thr_str = "_" + std::to_string(threshold);
    std::filesystem::path out_with_thr = outdir / (stem.string() + thr_str + ext.string());
    std::filesystem::path orig_out     = outdir / (stem.string() + "_original_gris" + thr_str + ext.string());

    cv::imwrite(out_with_thr.string(), out8);
    cv::imwrite(orig_out.string(), img8);

    // Mensajes informativos
    std::cout << "DCT compresión aplicada.\n";
    std::cout << "Umbral = " << threshold << "\n";
    std::cout << "Guardado reconstrucción: " << out_with_thr << "\n";
    std::cout << "Guardado original gris:  " << orig_out << "\n";
    std::cout << "Guardados coeficientes en carpeta output/.\n";

    return 0;
}
