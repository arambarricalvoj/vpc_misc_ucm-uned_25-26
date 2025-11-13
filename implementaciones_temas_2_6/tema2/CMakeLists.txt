cmake_minimum_required(VERSION 3.10)
project(tema2)

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
add_executable(pca transformacion_dominio_pca.cpp)
add_executable(wavelets transformacion_wavelets.cpp)
add_executable(dct transformacion_coseno.cpp)

# Enlaza librerías
target_link_libraries(pca ${OpenCV_LIBS})
target_link_libraries(wavelets ${OpenCV_LIBS})
target_link_libraries(dct ${OpenCV_LIBS})
