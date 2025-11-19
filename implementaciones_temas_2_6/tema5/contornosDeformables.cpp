/*
Contornos deformables (Snakes) con OpenCV
Minimización de energía: continuidad, curvatura y atracción a bordes.
Incluye adaptación dinámica de β_i según curvatura y gradiente.

Ejecución:
./snakes input.png output.png N sigma alpha beta gamma win T1 T2 T3 maxIter

Ejemplo:
./snakes lena.png result.png 100 1.0 0.8 0.6 1.0 7 5 0.02 20 200
 - N = 100        (número de puntos iniciales del contorno)
 - sigma = 1.0    (suavizado Gaussiano previo para gradiente)
 - alpha = 0.8    (peso continuidad)
 - beta = 0.6     (peso curvatura)
 - gamma = 1.0    (peso imagen)
 - win = 7        (vecindad m×m, m impar)
 - T1 = 5         (umbral de puntos movidos por iteración)
 - T2 = 0.02      (umbral de curvatura)
 - T3 = 20        (umbral de magnitud de gradiente)
 - maxIter = 200  (máximo número de iteraciones)
*/

Perfecto, lo que hiciste con las circunferencias fue expresar los parámetros en notación matemática dentro del texto. Te adapto ahora los parámetros de tus comandos de contornos deformables (snakes) en el mismo estilo:

latex
$N = 200;\; \sigma = 1.0;\; \alpha = 0.8;\; \beta = 0.6;\; \gamma = 1.0;\; 
\text{win} = 7;\; T_{1} = 5;\; T_{2} = 0.02;\; T_{3} = 20;\; \text{maxIter} = 200$
Otro ejemplo con los parámetros del segundo comando:

latex
$N = 100;\; \sigma = 1.0;\; \alpha = 0.3;\; \beta = 0.2;\; \gamma = 1.0;\; 
\text{win} = 5;\; T_{1} = 3;\; T_{2} = 0.02;\; T_{3} = 15;\; \text{maxIter} = 150$
Y para los casos de la imagen rojo_664.png:

latex
$N = 10;\; \sigma = 1.0;\; \alpha = 0.8;\; \beta = 0.6;\; \gamma = 1.0;\; 
\text{win} = 7;\; T_{1} = 5;\; T_{2} = 0.02;\; T_{3} = 20;\; \text{maxIter} = 200$
latex
$N = 100;\; \sigma = 1.0;\; \alpha = 0.8;\; \beta = 0.6;\; \gamma = 1.0;\; 
\text{win} = 7;\; T_{1} = 5;\; T_{2} = 0.02;\; T_{3} = 20;\; \text{maxIter} = 200$

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>

using namespace cv;
using namespace std;

// --- Energías locales ---
double E_cont(const Point2d& pi, const Point2d& p_prev) {
    return norm(pi - p_prev) * norm(pi - p_prev);
}

double E_curv(const Point2d& p_prev, const Point2d& pi, const Point2d& p_next) {
    Point2d v = p_prev - 2.0 * pi + p_next;
    return norm(v) * norm(v);
}

double E_imag(const Mat& gradMag, const Point2d& p) {
    int x = clamp<int>(lround(p.x), 0, gradMag.cols - 1);
    int y = clamp<int>(lround(p.y), 0, gradMag.rows - 1);
    float g = gradMag.at<float>(y, x);
    return g * g;
}

// --- Curvatura discreta ---
double curvature(const vector<Point2d>& P, int i) {
    int N = (int)P.size();
    int im1 = (i - 1 + N) % N;
    int ip1 = (i + 1) % N;
    Point2d c = P[im1] - 2.0 * P[i] + P[ip1];
    return norm(c);
}

// --- Inicialización circular ---
vector<Point2d> initCircleContour(const Mat& img, int N) {
    vector<Point2d> P(N);
    Point2d center(img.cols / 2.0, img.rows / 2.0);
    double r = min(img.cols, img.rows) * 0.4;
    for (int i = 0; i < N; ++i) {
        double t = 2.0 * CV_PI * i / N;
        P[i] = Point2d(center.x + r * cos(t), center.y + r * sin(t));
    }
    return P;
}

// --- Gradiente con suavizado ---
Mat gradientMagnitude(const Mat& gray, double sigma) {
    Mat smoothed;
    GaussianBlur(gray, smoothed, Size(), sigma, sigma);
    Mat gx, gy;
    Sobel(smoothed, gx, CV_32F, 1, 0, 3);
    Sobel(smoothed, gy, CV_32F, 0, 1, 3);
    Mat mag;
    magnitude(gx, gy, mag);
    return mag;
}

// --- Dibujar contorno ---
void drawContour(Mat& canvas, const vector<Point2d>& P, Scalar color) {
    for (size_t i = 0; i < P.size(); ++i) {
        Point p1((int)round(P[i].x), (int)round(P[i].y));
        Point p2((int)round(P[(i + 1) % P.size()].x), (int)round(P[(i + 1) % P.size()].y));
        line(canvas, p1, p2, color, 2);
    }
}

int main(int argc, char** argv) {
    if (argc < 13) {
        cerr << "Uso:\n"
             << "  " << argv[0] << " <imagen_entrada> <imagen_salida> N sigma alpha beta gamma win T1 T2 T3 maxIter\n";
        return -1;
    }

    string inputFile = argv[1];
    string outputFile = argv[2];
    int N = atoi(argv[3]);
    double sigma = atof(argv[4]);
    double alpha = atof(argv[5]);
    double beta = atof(argv[6]);
    double gamma = atof(argv[7]);
    int win = atoi(argv[8]);
    int T1 = atoi(argv[9]);
    double T2 = atof(argv[10]);
    double T3 = atof(argv[11]);
    int maxIter = atoi(argv[12]);

    Mat imgGray = imread(inputFile, IMREAD_GRAYSCALE);
    if (imgGray.empty()) {
        cerr << "No se pudo abrir la imagen: " << inputFile << endl;
        return -1;
    }

    Mat gradMag = gradientMagnitude(imgGray, sigma);
    vector<Point2d> P = initCircleContour(imgGray, N);
    vector<double> alpha_i(N, alpha), beta_i(N, beta), gamma_i(N, gamma);

    int winHalf = win / 2;
    int iter = 0;

    while (iter < maxIter) {
        int moved = 0;
        for (int i = 0; i < N; ++i) {
            int im1 = (i - 1 + N) % N;
            int ip1 = (i + 1) % N;
            double Emin = 1e12;
            Point2d best = P[i];

            for (int dy = -winHalf; dy <= winHalf; ++dy) {
                for (int dx = -winHalf; dx <= winHalf; ++dx) {
                    Point2d cand(P[i].x + dx, P[i].y + dy);
                    cand.x = clamp<double>(cand.x, 0, gradMag.cols - 1);
                    cand.y = clamp<double>(cand.y, 0, gradMag.rows - 1);

                    double E = alpha_i[i] * E_cont(cand, P[im1]) +
                               beta_i[i] * E_curv(P[im1], cand, P[ip1]) +
                               gamma_i[i] * E_imag(gradMag, cand);

                    if (E < Emin) {
                        Emin = E;
                        best = cand;
                    }
                }
            }
            if (norm(best - P[i]) > 1e-6) {
                P[i] = best;
                moved++;
            }
        }

        // Adaptación de beta_i
        for (int i = 0; i < N; ++i) {
            double c = curvature(P, i);
            int im1 = (i - 1 + N) % N;
            int ip1 = (i + 1) % N;
            int x = clamp<int>(round(P[i].x), 0, gradMag.cols - 1);
            int y = clamp<int>(round(P[i].y), 0, gradMag.rows - 1);
            double g = gradMag.at<float>(y, x);

            if (c > curvature(P, im1) && c > curvature(P, ip1) && c > T2 && g > T3) {
                beta_i[i] = 0.0;
            }
        }

        if (moved <= T1) break;
        iter++;
    }

    Mat colorOut;
    cvtColor(imgGray, colorOut, COLOR_GRAY2BGR);
    drawContour(colorOut, P, Scalar(0, 0, 255));
    imwrite(outputFile, colorOut);

    cout << "Contorno deformable guardado en: " << outputFile << endl;
    return 0;
}
