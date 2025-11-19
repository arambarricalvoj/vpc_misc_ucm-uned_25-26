cmake_minimum_required(VERSION 3.10)
project(Tema5)

# Establece el estándar de C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encuentra OpenCV
find_package(OpenCV REQUIRED)

# Incluye directorios
include_directories(
    ${OpenCV_INCLUDE_DIRS}
)

# Ejecutables
add_executable(hough hough.cpp kim_aggarwal.cpp filtroGaussiano.cpp)
add_executable(snakes contornosDeformables.cpp)

# Enlaza librerías
target_link_libraries(hough ${OpenCV_LIBS})
target_link_libraries(snakes ${OpenCV_LIBS})
