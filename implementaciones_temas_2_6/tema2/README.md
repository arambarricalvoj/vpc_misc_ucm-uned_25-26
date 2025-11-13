# Tema 2: Transformación
Implementaciones del tema 2.
## Ejecución

```bash
cd build/ && ./pca input_image output_image [var_ratio 0-1] [max_k]
cd build/ && ./wavelets input_image output_image [coeff_threshold]
cd build/ && ./dct input_image output_image [coeff_threshold]
```

Por ejemplo:
```bash
cd build/ && ./pca input_image output_image 0.95 200
cd build/ && ./wavelets propia_input.png propia_wavelet.png 0.05
cd build/ && ./dct propia_input.png propia_dct.png 0.35
```

## Compilación (ya está compilado)

```bash
rm -r CMakeCache.txt CMakeFiles/ cmake_install.cmake Makefile pca wavelets dct
cd build/ && cmake ..
make
```

---

<!-- 
# PCA para Compresión de Imágenes

Este proyecto implementa la **Transformada del dominio de Componentes Principales (PCA)** en C++ para la compresión de imágenes digitales en escala de grises. El objetivo es reducir la dimensionalidad de los datos conservando la mayor parte de la varianza.

---

## Flujo de trabajo

1. **Preparación de datos**  
   - Conversión a escala de grises  
   - Normalización en [0,1]  
   - Centrado por columnas  

2. **Cálculo de la covarianza**  
   - Se obtiene la matriz de covarianza de los datos centrados  

3. **Descomposición espectral**  
   - Autovalores y autovectores mediante `Eigen::SelfAdjointEigenSolver`  

4. **Selección de componentes**  
   - Se elige el número mínimo de componentes `k` que retienen la varianza deseada  
   - O se limita por un máximo `max_k`  

5. **Proyección y reconstrucción**  
   - Proyección en el subespacio de dimensión `k`  
   - Reconstrucción aproximada de la imagen  

---

## Ejecución

```bash
cd build/ && ./pca input_image output_image [var_ratio 0-1] [max_k]
```

Por ejemplo:
```bash
cd build/ &&./pca input_image output_image 0.95 200
```

## Compilación (ya está compilado, en caso de querer recompilar)

```bash
rm -r CMakeCache.txt CMakeFiles/ cmake_install.cmake Makefile pca
cd build/ && cmake ..
make
```

---

-->