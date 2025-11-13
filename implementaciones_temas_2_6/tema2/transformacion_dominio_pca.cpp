/* Transformada del dominio de Componentes Principales (PCA)
Objetivo: compresión de imágenes digitales

Flujo de trabajo: 
 - 1. preparación de datos (centrado y opcional estandarización), 
 - 2. cálculo de la covarianza, 
 - 3. descomposición en autovectores/autovalores,
 - 4. selección de componentes ,
 - 5. proyección/reconstrucción.

Ejecución:
./pca lena_input.png lena_output.png 0.99 200
    - input_image: nombre de la imagen a comprimir (tiene que estar en la carpeta input)
    - output_image: nombre del archivo de salida (se alamcena en la carpeta output)
    - var_ratio: umbral de varianza explicada (por defecto 0.95).
    - max_k: tope superior de componentes (opcional).

Bibliografía: https://www.geeksforgeeks.org/data-analysis/principal-component-analysis-pca/
https://www.datacamp.com/es/tutorial/covariance?dc_referrer=https%3A%2F%2Fwww.google.com%2F
*/

#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <iostream>
#include <vector>
#include <numeric>
#include <string>

/////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCIONES AUXILIARES
/////////////////////////////////////////////////////////////////////////////////////////////////
// Recorrer los autovalores acumulando varianza hasta alcanzar el umbral y devolver el k mínimo.
int select_k_by_variance(const Eigen::VectorXd& eigenvalues, double threshold_ratio) {
    // eigenvalues assumed sorted descending
    if (threshold_ratio < 0.0) {
        // No límite: selecciona todos los componentes
        return eigenvalues.size();
    }
    
    double total = eigenvalues.sum();
    double cumulative = 0.0;
    for (int i = 0; i < eigenvalues.size(); ++i) {
        cumulative += eigenvalues(i);
        if (cumulative / total >= threshold_ratio) {
            return i + 1;
        }
    }

    return eigenvalues.size();
}

// Dada una imagen matricial cv::Mat, convertirla a matriz Eigen con tamaño variable
// Se recorren todos los elementos para copiar los valores
Eigen::MatrixXd cvMatToEigen(const cv::Mat& mat) {
    Eigen::MatrixXd M(mat.rows, mat.cols);
    for (int r = 0; r < mat.rows; ++r) {
        const double* ptr = mat.ptr<double>(r);
        for (int c = 0; c < mat.cols; ++c) {
            M(r, c) = ptr[c];
        }
    }
    return M;
}

// Dada una matriz Eigen, convertirla a imagen matricial cv::Mat
// Se recorren todos los elementos para copiar los valores
cv::Mat eigenToCvMat(const Eigen::MatrixXd& M) {
    cv::Mat mat(M.rows(), M.cols(), CV_64F);
    for (int r = 0; r < M.rows(); ++r) {
        double* ptr = mat.ptr<double>(r);
        for (int c = 0; c < M.cols(); ++c) {
            ptr[c] = M(r, c);
        }
    }
    return mat;
}
/////////////////////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv) {
    
    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Leer argumentos o parámetros de llamada
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <input_image> <output_image> [var_ratio 0-1] [max_k]\n";
        std::cerr << "Ejemplo: " << argv[0] << " lena.png lena_pca.png 0.95 200\n";
        return 1;
    }
    std::string input_path = argv[1];
    std::string output_path = argv[2];
    double var_ratio = (argc >= 4) ? std::stod(argv[3]) : 0.95; // Por defecto 0.95
    int max_k = (argc >= 5) ? std::stoi(argv[4]) : -1;          // Por defecto -1 (sin límite)
    /////////////////////////////////////////////////////////////////////////////////////////////////

    
    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Cargar la imagen en escala de grises, convertirla a double para aumentar la precisión
    // y normalizar en [0, 1]
    cv::Mat img8 = cv::imread("input/" + input_path, cv::IMREAD_GRAYSCALE);
    if (img8.empty()) {
        std::cerr << "No se pudo cargar la imagen: " << input_path << "\n";
        return 1;
    }
    
    // Guardar también la original en escala de grises para comparar el resultado
    cv::imwrite("output/original_gris.png", img8);
    /////////////////////////////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////////////////////////////////    
    // 1.
    // Convertir la imagen a double y normalizar en [0, 1]
    cv::Mat img64;
    img8.convertTo(img64, CV_64F, 1.0 / 255.0);

    // Convertir la matriz de la imagen (cv::Mat) en una matriz de Eigen (Eigen::MatrixXd).
    // Una matriz Eigen es una matriz de números reales de tamaño variable de la librería
    // Eigen, especializada en álgebra lineal.
    Eigen::MatrixXd X = cvMatToEigen(img64);

    // Calcular la media de las columnas y obtenemos la matriz centrada
    Eigen::RowVectorXd col_means = X.colwise().mean();
    Eigen::MatrixXd X_centered = X.rowwise() - col_means;
    /////////////////////////////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////////////////////////////////
    // 2.
    // Calcular la covarianza y la descomposición espectral
    int n = X_centered.rows();
    Eigen::MatrixXd C = (X_centered.transpose() * X_centered) / double(n - 1);
    /////////////////////////////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////////////////////////////////
    // 3.
    // Calcular los autovectores y autovalores de la matriz de covarianza
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> es(C);
    if (es.info() != Eigen::Success) {
        std::cerr << "Fallo al calcular autovectores/autovalores.\n";
        return 1;
    }
    Eigen::VectorXd evals = es.eigenvalues();
    Eigen::MatrixXd evecs = es.eigenvectors();

    // Invertimos el orden de los autovalores, los queremos en orden ascendente
    // para tener primero los más importantes
    Eigen::VectorXd evals_desc(evals.size());
    Eigen::MatrixXd evecs_desc(evecs.rows(), evecs.cols());
    for (int i = 0; i < evals.size(); ++i) {
        evals_desc(i) = evals(evals.size() - 1 - i);
        evecs_desc.col(i) = evecs.col(evecs.cols() - 1 - i);
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////

    
    /////////////////////////////////////////////////////////////////////////////////////////////////
    // 4.
    // Selección de k componentes tal que retengan la varianza indicada.
    int k_var = select_k_by_variance(evals_desc, var_ratio);

    // Si hemos establecido un máximo de k, y las k que retienen la varianza
    // deseada superan el máximo, nos quedamos con el número máximo de k,
    // de lo contrario, nos quedamos con las k necesarias para retener la
    // varianza
    int k = (max_k > 0) ? std::min(max_k, k_var) : k_var;
    k = std::max(1, std::min(k, (int)evecs_desc.cols()));
    std::cout << "Componentes seleccionados k = " << k << " (umbral varianza = " << var_ratio << ")\n";
    Eigen::MatrixXd Wk = evecs_desc.leftCols(k);                // obtener los primeros k autovectores
    /////////////////////////////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////////////////////////////////
    // 5.
    // Proyectar: Z = X_centered * W_k    
    Eigen::MatrixXd Z = X_centered * Wk;                        

    // Reconstruir: X_hat = Z * (W_k)^T + means
    Eigen::MatrixXd X_hat = Z * Wk.transpose();    
    X_hat.rowwise() += col_means;

    // Escalar al rango [0, 1]
    for (int r = 0; r < X_hat.rows(); ++r) {
        for (int c = 0; c < X_hat.cols(); ++c) {
            X_hat(r, c) = std::min(1.0, std::max(0.0, X_hat(r, c)));
        }
    }

    // Convertir matriz a imagen y guardar
    cv::Mat out64 = eigenToCvMat(X_hat);
    cv::Mat out8;
    out64.convertTo(out8, CV_8U, 255.0);

    if (!cv::imwrite("output/" + output_path, out8)) {
        std::cerr << "No se pudo guardar la imagen: " << output_path << "\n";
        return 1;
    }

    std::cout << "Guardado: " << output_path << "\n";
    /////////////////////////////////////////////////////////////////////////////////////////////////


    return 0;
}