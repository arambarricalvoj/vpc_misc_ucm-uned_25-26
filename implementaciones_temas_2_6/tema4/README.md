# Tema 4: Extracción de bordes, regiones, puntos de interés
Implementaciones del tema 4.
## Ejecución

```bash
cd build/ && ./kim_aggarwal input.png output.png [sigma] [filterType] [kRel]
cd build/ && ./sobel input.png output.png [version="opencv"] [bin_threshold] [ksize]
cd build/ && ./sobel input.png output.png [version="manual" || version="opencv"] [bin_threshold]
cd build/ && ./harris input.png output_prefix [blockSize] [ksize] [k] [threshold]
```

Por ejemplo:
```bash
cd build/ && ./kim_aggarwal lena.png lena_output.png 0.5 8 10
cd build/ && ./sobel lena.png sobel_lena.png opencv 50 31
cd build/ && ./sobel lena.png sobel_lena.png manual 50 
cd build/ && ./harris reg1.jpg results 18 3 0.06 1.0
```

## Compilación (ya está compilado)

```bash
rm -r CMakeCache.txt CMakeFiles/ cmake_install.cmake Makefile kim_aggarwal sobel harris
cd build/ && cmake ..
make
```

---

