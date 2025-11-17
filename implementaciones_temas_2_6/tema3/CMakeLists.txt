cmake_minimum_required(VERSION 3.10)
project(tema3)

# Establece el estándar de C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encuentra OpenCV
find_package(OpenCV REQUIRED)

# Encuentra Eigen (solo encabezados)
find_path(EIGEN3_INCLUDE_DIR Eigen/Dense
    PATHS /usr/include/eigen3
)

# Incluye directorios
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
)

# Agrega el ejecutable
# add_executable(pca pca.cpp)
add_executable(ecualizacion ecualizacion.cpp)
add_executable(filtroGaussiano filtroGaussiano.cpp)
add_executable(registro registro.cpp)

# Enlaza librerías
target_link_libraries(ecualizacion ${OpenCV_LIBS})
target_link_libraries(filtroGaussiano ${OpenCV_LIBS})
target_link_libraries(registro ${OpenCV_LIBS})
