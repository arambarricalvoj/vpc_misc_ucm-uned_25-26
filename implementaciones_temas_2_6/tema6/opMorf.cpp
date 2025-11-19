/* Operaciones morfológicas en imágenes digitales
Objetivo: aplicar transformaciones clásicas de morfología matemática para análisis y 
          procesamiento de imágenes binarias o en escala de grises.

Flujo de trabajo:
 - 1. creación del elemento estructural (forma y tamaño),
 - 2. lectura de la imagen de entrada en escala de grises,
 - 3. aplicación de la operación morfológica seleccionada,
 - 4. escritura de la imagen de salida.

Operaciones disponibles:
    - dilatacion        → expansión de regiones blancas
    - erosion           → contracción de regiones blancas
    - apertura          → erosión seguida de dilatación (elimina ruido)
    - cierre            → dilatación seguida de erosión (rellena huecos)
    - bordes            → diferencia entre dilatación y erosión (resalta contornos)
    - vaciado           → resta de la imagen original y su erosión
    - engrosado         → unión de la imagen original y su dilatación
    - adelgazamiento    → intersección con el complemento de la erosión (reduce grosor)
    - llenado           → relleno de huecos internos mediante flood-fill y cierre

Ejecución:
./morfologia input.png output.png <operacion> [tamanoEE] [tipoEE] [umbral_binario]
    - input.png: imagen de entrada (escala de grises o binaria)
    - output.png: nombre del archivo de salida
    - operacion: operación morfológica a aplicar (ver lista arriba)
    - tamanoEE: tamaño del elemento estructural (por defecto 3)
    - tipoEE: forma del elemento estructural (completo, circular, cruz, diagpos, diagneg, vertical, horizontal)
    - umbral_binario: valor mínimo para binarización en operaciones como llenado

OJO! La memoria no está optimizada pues se realizan copias de las imágenes. Lo ideal sería alterar sobre las originales
*/


#include <filesystem>
#include <opencv2/opencv.hpp>
using namespace cv;
namespace fs = std::filesystem;

Mat crearElementoEstructural(int size, const std::string& shape) {
    Mat element = Mat::zeros(size, size, CV_8U);
    int r = size / 2;

    if (shape == "completo") {
        element = Mat::ones(size, size, CV_8U);

    } else if (shape == "ellipse" || shape == "circular") {
        Point center(r, r);
        circle(element, center, r, Scalar(1), -1);

    } else if (shape == "cruz") {
        for (int i = 0; i < size; i++) {
            element.at<uchar>(r, i) = 1;
            element.at<uchar>(i, r) = 1;
        }

    } else if (shape == "diagpos") {
        int thickness = size / 2;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (std::abs(i - j) <= thickness/2) {
                    element.at<uchar>(i,j) = 1;
                }
            }
        }

    } else if (shape == "diagneg") {
        int thickness = size / 2;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (std::abs((i + j) - (size - 1)) <= thickness/2) {
                    element.at<uchar>(i,j) = 1;
                }
            }
        }

    } else if (shape == "vertical") {
        int thickness = size / 2; // grosor de la franja
        for (int i = 0; i < size; i++) {
            for (int j = r - thickness/2; j <= r + thickness/2 && j < size; j++) {
                if (j >= 0) element.at<uchar>(i,j) = 1;
            }
        }

    } else if (shape == "horizontal") {
        int thickness = size / 2;
        for (int i = r - thickness/2; i <= r + thickness/2 && i < size; i++) {
            if (i >= 0) {
                for (int j = 0; j < size; j++) {
                    element.at<uchar>(i,j) = 1;
                }
            }
        }

    } else {
        element = Mat::ones(size, size, CV_8U); // por defecto rectángulo
    }

    return element;
}

// Dilatación
Mat dilatacion(const Mat& img, const Mat& element) {
    Mat out;
    // Mat element = getStructuringElement(MORPH_RECT, Size(size, size));
    dilate(img, out, element);
    return out;
}

Mat dilatacionManual(const Mat& img, const Mat& element) {
    Mat out = img.clone();
    // Mat element = getStructuringElement(MORPH_RECT, Size(size, size));

    int r = element.rows/2, c = element.cols/2;
    for (int y = 0; y < img.rows; y++) {
        for (int x = 0; x < img.cols; x++) {
            uchar maxVal = 0;
            for (int j=0; j<element.rows; j++) {
                for (int i=0; i<element.cols; i++) {
                    if (element.at<uchar>(j,i) > 0) {
                        int yy = y + j - r;
                        int xx = x + i - c;
                        if (yy>=0 && yy<img.rows && xx>=0 && xx<img.cols) {
                            maxVal = std::max(maxVal, img.at<uchar>(yy,xx));
                        }
                    }
                }
            }
            out.at<uchar>(y,x) = maxVal;
        }
    }
    return out;
}

// Erosión
Mat erosion(const Mat& img, const Mat& element) {
    Mat out;
    // Mat element = getStructuringElement(MORPH_RECT, Size(size, size));
    erode(img, out, element);
    return out;
}

Mat erosionManual(const Mat& img, const Mat& element) {
    Mat out = img.clone();
    // Mat element = getStructuringElement(MORPH_RECT, Size(size, size));

    int r = element.rows/2, c = element.cols/2;
    for (int y = 0; y < img.rows; y++) {
        for (int x = 0; x < img.cols; x++) {
            uchar minVal = 255;
            for (int j=0; j<element.rows; j++) {
                for (int i=0; i<element.cols; i++) {
                    if (element.at<uchar>(j,i) > 0) {
                        int yy = y + j - r;
                        int xx = x + i - c;
                        if (yy>=0 && yy<img.rows && xx>=0 && xx<img.cols) {
                            minVal = std::min(minVal, img.at<uchar>(yy,xx));
                        }
                    }
                }
            }
            out.at<uchar>(y,x) = minVal;
        }
    }
    return out;
}



// Apertura = erosión seguida de dilatación
Mat apertura(const Mat& img, const Mat& element) {
    Mat out;
    // Mat element = getStructuringElement(MORPH_RECT, Size(size, size));
    morphologyEx(img, out, MORPH_OPEN, element);
    return out;
}

Mat aperturaManual(const Mat& img, const Mat& element) {
    return dilatacionManual(erosionManual(img, element), element);
}


// Cierre = dilatación seguida de erosión
Mat cierre(const Mat& img, const Mat& element) {
    Mat out;
    //Mat element = getStructuringElement(MORPH_RECT, Size(size, size));
    morphologyEx(img, out, MORPH_CLOSE, element);
    return out;
}

Mat cierreManual(const Mat& img, const Mat& element) {
    return erosionManual(dilatacionManual(img, element), element);
}

// Bordes morfológicos = dilatación - erosión
Mat bordes(const Mat& img, const Mat& element) {
    Mat dil = dilatacion(img, element);
    Mat ero = erosion(img, element);
    Mat out;
    subtract(dil, ero, out);
    return out;
}

Mat bordesManual(const Mat& img, const Mat& element) {
    Mat dil = dilatacionManual(img, element);
    Mat ero = erosionManual(img, element);
    Mat out;
    subtract(dil, ero, out);
    return out;
}


// Vaciado = X - (X erosionado)
Mat vaciado(const Mat& img, const Mat& element) {
    Mat ero = erosion(img, element);
    Mat out;
    subtract(img, ero, out);
    return out;
}

Mat vaciadoManual(const Mat& img, const Mat& element) {
    Mat ero = erosionManual(img, element);
    Mat out;
    subtract(img, ero, out);
    return out;
}

// Engrosado = X ∪ (X dilatado)
Mat engrosado(const Mat& img, const Mat& element) {
    Mat dil = dilatacion(img, element);
    Mat out;
    bitwise_or(img, dil, out);
    return out;
}

Mat engrosadoManual(const Mat& img, const Mat& element) {
    Mat dil = dilatacionManual(img, element);
    Mat out;
    bitwise_or(img, dil, out);
    return out;
}

// Adelgazamiento = X ∩ (X erosionado)^c
Mat adelgazamiento(const Mat& img, const Mat& element) {
    Mat ero = erosion(img, element);
    Mat eroComp;
    bitwise_not(ero, eroComp);
    Mat out;
    bitwise_and(img, eroComp, out);
    return out;
}

Mat adelgazamientoManual(const Mat& img, const Mat& element) {
    Mat ero = erosionManual(img, element);
    Mat eroComp;
    bitwise_not(ero, eroComp);
    Mat out;
    bitwise_and(img, eroComp, out);
    return out;
}

Mat llenado(const Mat& img, const Mat& element, Point seed, int threshMin) {
// OJO!!! si hay esquina o borde en la semilla (0,0), entonces hay que dar otra!!
    // Semilla en el centro de la imagen
    /*Point seed(img.cols/2, img.rows/2);
    out = llenadoManual(img, seed, element); */

    // Binarizar la imagen
    cv::Mat binaria;
    cv::threshold(img, binaria, threshMin, 255, cv::THRESH_BINARY);

    // Cerrar huecos en los bordes para asegurar contornos cerrados
    cv::morphologyEx(binaria, binaria, cv::MORPH_CLOSE, element);
    cv::imshow("Cierre", binaria);
    cv::waitKey(0);

    // Invertir la imagen
    cv::Mat inv;
    cv::bitwise_not(binaria, inv);

    // FloodFill desde el exterior
    cv::Mat flood = inv.clone();
    cv::floodFill(flood, seed, cv::Scalar(255));

    /*
    recorrido de píxeles conectados (como un BFS o DFS en grafos) a partir de una semilla.
    
    Empiezas en un punto semilla dentro de la región que quieres rellenar.

    Compruebas si ese píxel cumple la condición (por ejemplo, valor = 0 → fondo).

    Lo marcas con el nuevo valor (por ejemplo, 255 → relleno).

    Añades sus vecinos (4-conectividad o 8-conectividad) a una pila o cola.

    Repites hasta que no queden píxeles por visitar.*/

    // Invertir de nuevo
    cv::Mat flood_inv;
    cv::bitwise_not(flood, flood_inv);

    // Combinar con la original: los huecos quedan rellenos
    Mat filled;
    filled = (binaria | flood_inv);

    return filled;
}

Mat llenadoManual(const Mat& img, const Mat& element, Point seed, int threshMin) {
    // Paso 1: binarizar por seguridad
    Mat binaria;
    threshold(img, binaria, 128, 255, THRESH_BINARY);

    // Paso 2: aplicar cierre morfológico manual para cerrar contornos
    Mat cerrada = cierreManual(binaria, element);

    // Paso 3: máscara = fondo (inverso de la imagen cerrada)
    Mat mask;
    bitwise_not(cerrada, mask);

    // Paso 4: inicializar semilla
    Mat Xk = Mat::zeros(img.size(), CV_8UC1);
    Xk.at<uchar>(seed) = 255;

    // Paso 5: iterar dilatación restringida
    while (true) {
        Mat Xk1 = Xk.clone();
        Xk1 = dilatacionManual(Xk1, element);   // dilatación
        bitwise_and(Xk1, mask, Xk1);            // restringido al fondo

        Mat diff;
        compare(Xk1, Xk, diff, CMP_NE);
        if (countNonZero(diff) == 0) break;     // condición de parada

        Xk = Xk1;
    }

    // Paso 6: unir relleno con la imagen cerrada
    Mat filled;
    bitwise_or(Xk, cerrada, filled);

    return filled;
}


static void printUsage(const char* prog) {
    std::cerr << "Uso:\n"
              << prog << " <imagen_entrada> <imagen_salida> <operacion> [tamanoEE] [tipoEE]\n"
              << "  <operacion>: dilatacion | erosion | apertura | cierre | bordes | vaciado | engrosado | adelgazamiento | llenado\n"
              << "  <tipoEE>: completo | circular | cruz | diagpos | diagneg | vertical | horizontal\n"
              << "Ejemplo:\n"
              << "  " << prog << " input.png out.png dilatacion 5\n";
}

int main(int argc, char** argv) {
    if (argc < 4) { printUsage(argv[0]); return -1; }
    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    std::string operacion = argv[3];
    int size = (argc > 4) ? atoi(argv[4]) : 3;
    std::string shape = argv[5];
    int binaryMinthreshold = std::stoi(argv[6]);

    // Mat element = getStructuringElement(MORPH_RECT, Size(size, size));
    Mat element = crearElementoEstructural(size, shape);

    // Leer siempre en escala de grises
    Mat img = imread(inputFile, IMREAD_GRAYSCALE);
    if (img.empty()) {
        std::cerr << "No se pudo abrir la imagen\n";
        return -1;
    }

    Mat out;

    if (operacion == "dilatacion") out = dilatacionManual(img, element);
    else if (operacion == "erosion") out = erosionManual(img, element);
    else if (operacion == "apertura") out = aperturaManual(img, element);
    else if (operacion == "cierre") out = cierreManual(img, element);
    else if (operacion == "bordes") out = bordesManual(img, element);
    else if (operacion == "vaciado") out = vaciadoManual(img, element);
    else if (operacion == "engrosado") out = engrosadoManual(img, element);
    else if (operacion == "adelgazamiento") out = adelgazamientoManual(img, element);
    else if (operacion == "llenado") { out = llenado(img, element, cv::Point(0,0), binaryMinthreshold);}

    else {
        std::cerr << "Operación no reconocida: " << operacion << std::endl;
        return -1;
    }


    if (!imwrite(outputFile, out)) {std::cerr << "No se pudo guardar la imagen en: " << outputFile << std::endl;} 
    else {std::cout << "Imagen guardada en: " << outputFile << std::endl;}

    return 0;
}


