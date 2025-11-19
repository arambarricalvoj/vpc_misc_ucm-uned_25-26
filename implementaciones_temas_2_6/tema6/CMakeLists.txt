cmake_minimum_required(VERSION 3.10)
project(Tema6)

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
add_executable(opMorf opMorf.cpp)

# Enlaza librerías
target_link_libraries(opMorf ${OpenCV_LIBS})
