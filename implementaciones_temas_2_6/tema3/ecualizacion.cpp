/*
Transformaciones de intensidades basadas en la CDF del histograma
Objetivo: aplicar diferentes métodos de ecualización/transformación de contraste
          sobre imágenes digitales en escala de grises.

Flujo de trabajo:
 - 1. Cargar imagen en escala de grises.
 - 2. Calcular histograma normalizado (PDF).
 - 3. Calcular función de distribución acumulada (CDF).
 - 4. Aplicar transformación de intensidades según el método elegido:
       * uniforme
       * exponencial (λ)
       * hipercúbica (n)
       * log-hiperbólica (α)
       * Rayleigh (σ)
 - 5. Guardar imagen transformada y sus histogramas comparativos.

Ejecución:
./ecualizacion input.png output.png metodo [param]
    - input_image: nombre de la imagen de entrada.
    - output_image: nombre base del archivo de salida.
    - metodo: tipo de transformación (uniforme, exponencial, hipercubica, loghiper, rayleigh).
    - param: parámetro adicional requerido por algunos métodos:
        * exponencial → lambda
        * hipercubica → n
        * loghiper    → alpha
        * rayleigh    → sigma

Ejemplo:
./ecualizacion bajoContraste1.png bajoConstraste1_out.png uniforme
./ecualizacion bajoContraste1.png bajoConstraste1_out.png exponencial 0.5

Salida:
 - Imagen transformada: <output_image>_<metodo>.png
 - Histograma original: <output_image>_hist_original.png
 - Histograma transformado: <output_image>_hist_ecualizado_<metodo>.png
*/

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

using namespace cv;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// FUNCIONES AUXILIARES
////////////////////////////////////////////////////////////////////////////////

// Aplicar una transformación pixel a pixel sobre la imagen
Mat aplicarTransformacion(const Mat& img, function<int(int)> f) {
    Mat salida = img.clone();
    for (int y = 0; y < img.rows; y++) {
        for (int x = 0; x < img.cols; x++) {
            int val = img.at<uchar>(y, x);
            salida.at<uchar>(y, x) = saturate_cast<uchar>(f(val));
        }
    }
    return salida;
}

// Función para calcular y dibujar histograma
void guardarHistograma(const Mat& img, const string& filename) {
    // Calcular histograma
    int histSize = 256;
    float range[] = {0, 256};
    const float* histRange = {range};
    Mat hist;
    calcHist(&img, 1, 0, Mat(), hist, 1, &histSize, &histRange);

    // Normalizar para mostrar
    int hist_w = 512, hist_h = 400;
    int bin_w = cvRound((double) hist_w / histSize);
    Mat histImage(hist_h + 50, hist_w + 60, CV_8UC3, Scalar(255, 255, 255));

    normalize(hist, hist, 0, hist_h, NORM_MINMAX);

    // Dibujar histograma como línea continua
    for (int i = 1; i < histSize; i++) {
        line(histImage,
             Point(bin_w * (i - 1) + 50, hist_h - cvRound(hist.at<float>(i - 1))),
             Point(bin_w * (i) + 50, hist_h - cvRound(hist.at<float>(i))),
             Scalar(0, 0, 0), 2, 8, 0);
    }

    // Dibujar ejes
    line(histImage, Point(50, 0), Point(50, hist_h), Scalar(0,0,0), 2); // eje Y
    line(histImage, Point(50, hist_h), Point(hist_w+50, hist_h), Scalar(0,0,0), 2); // eje X

    // Marcas en X (niveles de gris cada 50)
    for (int i = 0; i <= 255; i += 50) {
        int x = cvRound((double)i * hist_w / 256.0) + 50;
        line(histImage, Point(x, hist_h), Point(x, hist_h+5), Scalar(0,0,0), 1);
        putText(histImage, to_string(i), Point(x-10, hist_h+20),
                FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0,0,0), 1);
    }

    // Marcas en Y (frecuencia normalizada cada 100 px aprox)
    for (int j = 0; j <= hist_h; j += 100) {
        line(histImage, Point(45, hist_h - j), Point(50, hist_h - j), Scalar(0,0,0), 1);
        putText(histImage, to_string(j), Point(5, hist_h - j + 5),
                FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0,0,0), 1);
    }

    imwrite(filename, histImage);
}

////////////////////////////////////////////////////////////////////////////////
// PROGRAMA PRINCIPAL
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
    if (argc < 4) {
        cerr << "Uso: " << argv[0]
             << " <input_image> <output_image> <metodo> [param]\n";
        cerr << "Metodos disponibles:\n";
        cerr << "   uniforme\n";
        cerr << "   exponencial <lambda>\n";
        cerr << "   hipercubica <n>\n";
        cerr << "   loghiper <alpha>\n";
        cerr << "   rayleigh <sigma>\n";
        return 1;
    }

    string input_path = argv[1];
    string output_path = argv[2];
    string metodo = argv[3];

    // Cargar imagen en escala de grises
    Mat img = imread(input_path, IMREAD_GRAYSCALE);
    if (img.empty()) {
        cerr << "No se pudo cargar la imagen: " << input_path << endl;
        return -1;
    }

    int L = 256; // número de niveles de gris
    Mat img_salida;

    // --- Paso 1: calcular histograma normalizado (PDF) ---
    // PDF: Función de Densidad de Probabilidad
    vector<int> hist(L, 0);
    for (int y = 0; y < img.rows; y++) {
        for (int x = 0; x < img.cols; x++) {
            int val = img.at<uchar>(y, x);
            hist[val]++;
        }
    }

    int totalPixeles = img.rows * img.cols;
    vector<double> pdf(L, 0.0);
    for (int i = 0; i < L; i++) {
        pdf[i] = (double)hist[i] / totalPixeles;
    }

    // --- Paso 2: calcular CDF acumulada ---
    vector<double> cdf(L, 0.0);
    cdf[0] = pdf[0];
    for (int i = 1; i < L; i++) {
        cdf[i] = cdf[i-1] + pdf[i];
    }

    // --- Paso 3: aplicar transformaciones usando la CDF ---
    // Función de distribución acumulada
    if (metodo == "uniforme") {
        auto uniforme = [&](int r) {
            return (int)((L - 1) * cdf[r]);
        };
        img_salida = aplicarTransformacion(img, uniforme);
    }
    else if (metodo == "exponencial") {
        if (argc < 5) {
            cerr << "Falta parámetro lambda\n";
            return -1;
        }
        double lambda = atof(argv[4]);
        auto exponencial = [&](int r) {
            double Pr = cdf[r];
            return (int)((L - 1) * (1 - exp(-lambda * Pr)));
        };
        img_salida = aplicarTransformacion(img, exponencial);
    }
    else if (metodo == "hipercubica") {
        if (argc < 5) {
            cerr << "Falta parámetro n\n";
            return -1;
        }
        int n = atoi(argv[4]);
        auto hipercubica = [&](int r) {
            double Pr = cdf[r];
            return (int)((L - 1) * pow(Pr, n));
        };
        img_salida = aplicarTransformacion(img, hipercubica);
    }
    else if (metodo == "loghiper") {
        if (argc < 5) {
            cerr << "Falta parámetro alpha\n";
            return -1;
        }
        double alpha = atof(argv[4]);
        auto logHiper = [&](int r) {
            double Pr = cdf[r];
            return (int)((L - 1) * log(1 + alpha * Pr) / log(1 + alpha));
        };
        img_salida = aplicarTransformacion(img, logHiper);
    }
    else if (metodo == "rayleigh") {
        if (argc < 5) {
            cerr << "Falta parámetro sigma\n";
            return -1;
        }
        double sigma = atof(argv[4]);
        auto rayleigh = [&](int r) {
            double Pr = cdf[r]; // valor acumulado normalizado en [0,1]
            double numerador   = 1.0 - exp(-(Pr * Pr) / (2.0 * sigma * sigma));
            double denominador = 1.0 - exp(-1.0 / (2.0 * sigma * sigma));
            return (int)((L - 1) * (numerador / denominador));
        };
        img_salida = aplicarTransformacion(img, rayleigh);
    }
    else {
        cerr << "Método no reconocido: " << metodo << endl;
        return -1;
    }

    // --- Paso 4: guardar resultados ---
    // Guardar imagen transformada
    imwrite(output_path + "_" + metodo + ".png", img_salida);

    // Guardar histogramas comparativos
    string baseName = output_path.substr(0, output_path.find_last_of('.'));
    guardarHistograma(img, baseName + "_hist_original.png");
    guardarHistograma(img_salida, baseName + "_hist_ecualizado_" + metodo + ".png");

    cout << "Proceso completado. Imagen y histogramas guardados." << endl;
    return 0;
}
