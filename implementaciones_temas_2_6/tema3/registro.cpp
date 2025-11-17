/*
Registro automático entre imágenes usando ORB + Homografía
Objetivo: alinear una imagen móvil con respecto a una imagen de referencia
          mediante detección de puntos clave, emparejamiento y cálculo de
          homografía.

Flujo de trabajo:
 1. Leer imágenes de referencia y móvil.
 2. Detectar puntos clave y descriptores con ORB.
 3. Emparejar descriptores con BFMatcher.
 4. Ordenar coincidencias por distancia y conservar un porcentaje de las mejores.
 5. Calcular homografía con RANSAC.
 6. Aplicar transformación (warpPerspective) para registrar la imagen móvil.
 7. Guardar resultado.

Ejecución:
./registro ref.png mov.png [ratio]
    - ref.png: imagen de referencia.
    - mov.png: imagen a registrar.
    - ratio: porcentaje de coincidencias a conservar (ej. 0.15 = 15%). Por defecto 0.15.

Salida:
 - Imagen registrada guardada como 'registrada.png'.

Notas:
 - La homografía permite manejar rotación, escala, traslación y perspectiva.
*/

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout << "Uso: " << argv[0] << " <imagen_ref> <imagen_mov> [ratio]\n";
        return -1;
    }

    std::string refPath = argv[1];
    std::string movPath = argv[2];
    double ratio = (argc >= 4) ? std::stod(argv[3]) : 0.15; // porcentaje de coincidencias

    // 1. Cargar imágenes en escala de grises
    cv::Mat imgRef = cv::imread(refPath, cv::IMREAD_GRAYSCALE);
    cv::Mat imgMov = cv::imread(movPath, cv::IMREAD_GRAYSCALE);
    if (imgRef.empty() || imgMov.empty()) {
        std::cerr << "Error al cargar imágenes\n";
        return -1;
    }

    // 2. Detectar puntos clave y descriptores con ORB
    cv::Ptr<cv::Feature2D> detector = cv::ORB::create(5000);
    std::vector<cv::KeyPoint> kpRef, kpMov;
    cv::Mat descRef, descMov;
    detector->detectAndCompute(imgRef, cv::noArray(), kpRef, descRef);
    detector->detectAndCompute(imgMov, cv::noArray(), kpMov, descMov);

    // 3. Emparejar descriptores con BFMatcher
    cv::BFMatcher matcher(cv::NORM_HAMMING);
    std::vector<cv::DMatch> matches;
    matcher.match(descRef, descMov, matches);

    // 4. Ordenar por distancia y conservar las mejores coincidencias
    std::sort(matches.begin(), matches.end(),
              [](const cv::DMatch& a, const cv::DMatch& b) {
                  return a.distance < b.distance;
              });
    int numGoodMatches = matches.size() * ratio;
    matches.erase(matches.begin() + numGoodMatches, matches.end());

    // 5. Extraer puntos correspondientes
    std::vector<cv::Point2f> ptsRef, ptsMov;
    for (auto& m : matches) {
        ptsRef.push_back(kpRef[m.queryIdx].pt);
        ptsMov.push_back(kpMov[m.trainIdx].pt);
    }

    // 6. Calcular homografía con RANSAC
    cv::Mat H = cv::findHomography(ptsMov, ptsRef, cv::RANSAC);

    // 7. Aplicar transformación
    cv::Mat imgReg;
    cv::warpPerspective(imgMov, imgReg, H, imgRef.size());

    // Guardar resultado
    cv::imwrite("registrada.png", imgReg);
    std::cout << "Registro completado. Imagen guardada en 'registrada.png'\n";

    return 0;
}
