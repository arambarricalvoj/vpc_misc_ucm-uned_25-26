/* Transformada de Hough
Objetivo: detectar formas geométricas (líneas y circuferencias) en imágenes

Ejecución:
./hough original.png out_lines.png 1.0 8 0.5 lines --thetaBins 180 --rhoStep 1 --lineVotes 80 --maxLines 10
    - input_image: ruta de la imagen de entrada
    - output_image: ruta de la imagen de salida
    - sigma: desviación estándar del filtro gaussiano para la aproximación de Kim y Aggarwal
    - filtro_laplaciano: tipo de máscara laplaciana (4 para 4-conexo, 6 para ponderado, 8 para 8-conexo)
    - kRel: umbral relativo para la detección de cruces por cero en Kim y Aggarwal
    - modo: lines o circles.

    Parámetros adicionales para rectas (modo = lines)
    - thetaBins N: número de divisiones del ángulo theta
    - rhoStep R: paso en la distancia ro
    - lineVotes T: umbral mínimo de votos para aceptar una recta
    - lineNMS K: supresión de no-máximos
    - maxLines M: número máximo de rectas a dibujar

    Parámetros adicionales para círculos (modo = circles)
    - rMin A: radio mínimo
    - rMax B: radio máximo
    - rStep S: incremento de radio
    - angleBins K: número de divisiones angulares
    - circleVotes T: umbral mínimo de votos para aceptar un círculo
    - circleNMS K: supresión de no-máximos
    - maxCircles M: número máximo de circunferencias a dibujar
*/

#include "kim_aggarwal.hpp"
#include "filtroGaussiano.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

using namespace cv;
using namespace std;

// -----------------------------
// HOUGH PARA RECTAS
// -----------------------------
struct LineParam {
    float rho;
    float theta;
    int votes;
};

void houghLinesCustom(const Mat& edges, vector<LineParam>& lines,
                      int thetaBins = 180, float rhoStep = 1.0f,
                      int voteThreshold = 100, int nmsNeighborhood = 3)
{
    // Asegurar binario
    CV_Assert(edges.type() == CV_8U);
    int rows = edges.rows, cols = edges.cols;

    // Rango de rho: [-rhoMax, rhoMax)
    float rhoMax = std::hypot(rows, cols);
    int rhoBins = int(std::ceil((2.0f * rhoMax) / rhoStep));
    // Acumulador: (rhoIdx, thetaIdx)
    Mat accumulator = Mat::zeros(rhoBins, thetaBins, CV_32S);

    // Precompute seno/coseno
    vector<float> cosTable(thetaBins), sinTable(thetaBins);
    for (int t = 0; t < thetaBins; ++t) {
        float theta = (float)CV_PI * (float(t) / thetaBins); // [0, π)
        cosTable[t] = std::cos(theta);
        sinTable[t] = std::sin(theta);
    }

    // Votación
    for (int y = 0; y < rows; ++y) {
        const uchar* rowPtr = edges.ptr<uchar>(y);
        for (int x = 0; x < cols; ++x) {
            if (rowPtr[x] == 0) continue; // solo píxeles de borde
            for (int t = 0; t < thetaBins; ++t) {
                float rho = x * cosTable[t] + y * sinTable[t];
                int rhoIdx = int(std::floor((rho + rhoMax) / rhoStep));
                if (rhoIdx >= 0 && rhoIdx < rhoBins) {
                    accumulator.at<int>(rhoIdx, t) += 1;
                }
            }
        }
    }

    // Supresión de no-máximos + umbral de votos
    // Buscar picos locales en el acumulador
    for (int r = 0; r < rhoBins; ++r) {
        for (int t = 0; t < thetaBins; ++t) {
            int v = accumulator.at<int>(r, t);
            if (v < voteThreshold) continue;

            bool isPeak = true;
            for (int dr = -nmsNeighborhood; dr <= nmsNeighborhood && isPeak; ++dr) {
                for (int dt = -nmsNeighborhood; dt <= nmsNeighborhood; ++dt) {
                    if (dr == 0 && dt == 0) continue;
                    int rr = r + dr;
                    int tt = (t + dt + thetaBins) % thetaBins;
                    if (rr < 0 || rr >= rhoBins) continue;
                    if (accumulator.at<int>(rr, tt) > v) {
                        isPeak = false;
                        break;
                    }
                }
            }
            if (isPeak) {
                float rho = -rhoMax + r * rhoStep;
                float theta = (float)CV_PI * (float(t) / thetaBins);
                lines.push_back(LineParam{rho, theta, v});
            }
        }
    }

    // Ordenar por votos descendente
    std::sort(lines.begin(), lines.end(), [](const LineParam& a, const LineParam& b) {
        return a.votes > b.votes;
    });
}

void drawLines(Mat& colorImg, const vector<LineParam>& lines, int maxLines = 10, Scalar color = Scalar(0,0,255)) {
    int rows = colorImg.rows, cols = colorImg.cols;
    int count = std::min<int>(maxLines, lines.size());
    for (int i = 0; i < count; ++i) {
        float rho = lines[i].rho;
        float theta = lines[i].theta;
        double a = std::cos(theta), b = std::sin(theta);
        double x0 = a * rho, y0 = b * rho;
        Point pt1, pt2;
        // puntos alejados para dibujar la línea a través de la imagen
        pt1.x = cvRound(x0 + 10000 * (-b));
        pt1.y = cvRound(y0 + 10000 * (a));
        pt2.x = cvRound(x0 - 10000 * (-b));
        pt2.y = cvRound(y0 - 10000 * (a));
        line(colorImg, pt1, pt2, color, 2, LINE_AA);
    }
}

// -----------------------------
// HOUGH PARA CIRCUNFERENCIAS (votación angular)
// -----------------------------
struct CircleParam {
    int a; // centro x
    int b; // centro y
    int r; // radio
    int votes;
};

void houghCirclesCustom(const Mat& edges, vector<CircleParam>& circles,
                        int rMin, int rMax, int rStep,
                        int angleBins = 360, int voteThreshold = 80,
                        int nmsNeighborhood = 2)
{
    CV_Assert(edges.type() == CV_8U);
    int rows = edges.rows, cols = edges.cols;

    if (rMin < 1) rMin = 1;
    if (rMax < rMin) rMax = rMin;

    // Para cada radio, creamos acumulador de centros (a,b)
    // Para eficiencia, reusamos una sola matriz y detectamos picos por radio.
    vector<float> cosTable(angleBins), sinTable(angleBins);
    for (int k = 0; k < angleBins; ++k) {
        float ang = (2.0f * (float)CV_PI) * (float(k) / angleBins);
        cosTable[k] = std::cos(ang);
        sinTable[k] = std::sin(ang);
    }

    Mat accumulator; // se dimensionará por radio
    accumulator.create(rows, cols, CV_32S);

    // Lista de píxeles de borde para acelerar
    vector<Point> edgePoints;
    edgePoints.reserve(rows * cols / 10);
    for (int y = 0; y < rows; ++y) {
        const uchar* rp = edges.ptr<uchar>(y);
        for (int x = 0; x < cols; ++x) {
            if (rp[x]) edgePoints.emplace_back(x, y);
        }
    }

    for (int r = rMin; r <= rMax; r += rStep) {
        accumulator.setTo(Scalar(0));

        // Votación: para cada punto de borde y cada ángulo, votar centro posible
        // a = x - r cos(theta), b = y - r sin(theta)
        for (const auto& p : edgePoints) {
            int x = p.x, y = p.y;
            for (int k = 0; k < angleBins; ++k) {
                int a = cvRound(x - r * cosTable[k]);
                int b = cvRound(y - r * sinTable[k]);
                if (a >= 0 && a < cols && b >= 0 && b < rows) {
                    accumulator.at<int>(b, a) += 1;
                }
            }
        }

        // Supresión de no-máximos y umbral
        for (int by = 0; by < rows; ++by) {
            for (int ax = 0; ax < cols; ++ax) {
                int v = accumulator.at<int>(by, ax);
                if (v < voteThreshold) continue;
                bool isPeak = true;
                for (int dy = -nmsNeighborhood; dy <= nmsNeighborhood && isPeak; ++dy) {
                    for (int dx = -nmsNeighborhood; dx <= nmsNeighborhood; ++dx) {
                        if (dx == 0 && dy == 0) continue;
                        int yy = by + dy, xx = ax + dx;
                        if (yy < 0 || yy >= rows || xx < 0 || xx >= cols) continue;
                        if (accumulator.at<int>(yy, xx) > v) {
                            isPeak = false;
                            break;
                        }
                    }
                }
                if (isPeak) {
                    circles.push_back(CircleParam{ax, by, r, v});
                }
            }
        }
    }

    // Ordenar por votos
    std::sort(circles.begin(), circles.end(), [](const CircleParam& a, const CircleParam& b) {
        return a.votes > b.votes;
    });
}

void drawCircles(Mat& colorImg, const vector<CircleParam>& circles, int maxCircles = 10, Scalar color = Scalar(0,255,0)) {
    int count = std::min<int>(maxCircles, circles.size());
    for (int i = 0; i < count; ++i) {
        circle(colorImg, Point(circles[i].a, circles[i].b), circles[i].r, color, 2, LINE_AA);
        // opcional: dibujar centro
        circle(colorImg, Point(circles[i].a, circles[i].b), 2, Scalar(0,0,0), FILLED, LINE_AA);
    }
}

// -----------------------------
// MAIN con menú/argumentos
// -----------------------------
static void printUsage(const char* prog) {
    std::cerr << "Uso:\n"
              << prog << " <imagen_entrada> <imagen_salida> <sigma> <filtro_laplaciano> <kRel> <modo> [opciones_hough]\n"
              << "  <modo>: lines | circles | both\n"
              << "Opciones Hough (rectas): --thetaBins N --rhoStep R --lineVotes T --lineNMS K --maxLines M\n"
              << "Opciones Hough (circulos): --rMin A --rMax B --rStep S --angleBins K --circleVotes T --circleNMS K --maxCircles M\n"
              << "Ejemplo:\n"
              << "  " << prog << " input.png out.png 1.0 8 0.5 lines --thetaBins 180 --rhoStep 1 --lineVotes 80 --maxLines 10\n"
              << "  " << prog << " input.png out.png 1.0 8 0.5 circles --rMin 10 --rMax 80 --rStep 2 --angleBins 360 --circleVotes 60 --maxCircles 8\n";
}

int main(int argc, char** argv) {
    if (argc < 7) {
        printUsage(argv[0]);
        return -1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    double sigma = atof(argv[3]);
    int lap_filter = atoi(argv[4]);
    double kRel = atof(argv[5]);
    std::string mode = argv[6]; // "lines" | "circles" | "both"

    Mat img = imread(inputFile, IMREAD_GRAYSCALE);
    if (img.empty()) {
        std::cerr << "No se pudo abrir la imagen: " << inputFile << std::endl;
        return -1;
    }

    // 1) Bordes
    Mat edges = zeroCrossingLoG(img, lap_filter, sigma, kRel);
    cv::imshow("Imagen original", edges);
    cv::imshow("Bordes tras Laplaciana", edges);
    cv::waitKey(0); // espera a que pulses una tecla
    // Guardar la imagen de bordes
    if (!cv::imwrite("lap.png", edges)) {
        std::cerr << "No se pudo guardar la imagen Laplaciana en: " << "lap.png" << std::endl;
    } else {
        std::cout << "Imagen Laplaciana guardada en: " << "lap.png" << std::endl;
    }


    // Parámetros por defecto Hough lines
    int thetaBins = 180;
    float rhoStep = 1.0f;
    int lineVotes = 100;
    int lineNMS = 3;
    int maxLines = 10;

    // Parámetros por defecto Hough circles
    int rMin = 10, rMax = 80, rStep = 2;
    int angleBins = 360;
    int circleVotes = 80;
    int circleNMS = 2;
    int maxCircles = 8;

    // Parse opciones adicionales
    for (int i = 7; i < argc; ++i) {
        std::string arg = argv[i];
        auto getVal = [&](int& i)->const char* { if (i+1 < argc) return argv[++i]; else return ""; };
        if (arg == "--thetaBins") thetaBins = atoi(getVal(i));
        else if (arg == "--rhoStep") rhoStep = (float)atof(getVal(i));
        else if (arg == "--lineVotes") lineVotes = atoi(getVal(i));
        else if (arg == "--lineNMS") lineNMS = atoi(getVal(i));
        else if (arg == "--maxLines") maxLines = atoi(getVal(i));
        else if (arg == "--rMin") rMin = atoi(getVal(i));
        else if (arg == "--rMax") rMax = atoi(getVal(i));
        else if (arg == "--rStep") rStep = atoi(getVal(i));
        else if (arg == "--angleBins") angleBins = atoi(getVal(i));
        else if (arg == "--circleVotes") circleVotes = atoi(getVal(i));
        else if (arg == "--circleNMS") circleNMS = atoi(getVal(i));
        else if (arg == "--maxCircles") maxCircles = atoi(getVal(i));
        else {
            std::cerr << "Argumento desconocido: " << arg << std::endl;
        }
    }

    // 2) Ejecutar Hough
    Mat colorOut;
    cvtColor(img, colorOut, COLOR_GRAY2BGR);

    if (mode == "lines" || mode == "both") {
        vector<LineParam> lines;
        houghLinesCustom(edges, lines, thetaBins, rhoStep, lineVotes, lineNMS);
        drawLines(colorOut, lines, maxLines, Scalar(0,0,255));
        std::cout << "Rectas detectadas: " << lines.size() << std::endl;
    }

    if (mode == "circles" || mode == "both") {
        vector<CircleParam> circles;
        houghCirclesCustom(edges, circles, rMin, rMax, rStep, angleBins, circleVotes, circleNMS);
        drawCircles(colorOut, circles, maxCircles, Scalar(0,255,0));
        std::cout << "Circunferencias detectadas: " << circles.size() << std::endl;
    }

    // 3) Guardar resultados
    if (!imwrite(outputFile, colorOut)) {
        std::cerr << "No se pudo guardar la imagen de salida en: " << outputFile << std::endl;
        return -1;
    }

    // 4) Guardar bordes intermedios (opcional)
    std::string edgesOut = "edges_zerox_log.png";
    imwrite(edgesOut, edges);
    std::cout << "Imagen de bordes guardada en: " << edgesOut << std::endl;
    std::cout << "Resultado Hough guardado en: " << outputFile << std::endl;

    return 0;
}
