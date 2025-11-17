# Tema 4: Extracción de bordes, regiones, puntos de interés
Implementaciones del tema 4.
## Ejecución

```bash
cd build/ && ./kim_aggarwal input.png output.png [sigma] [filterType] [kRel]
cd build/ && ./sobel input.png output.png [version="opencv"] [bin_threshold] [ksize]
cd build/ && ./sobel input.png output.png [version="manuak"] [bin_threshold]
```

Por ejemplo:
```bash
cd build/ && ./kim_aggarwal lena.png lena_output.png 0.5 8 10
cd build/ && ./sobel lena.png sobel_lena.png opencv 50 31
cd build/ && ./sobel lena.png sobel_lena.png manual 50 
```

## Compilación (ya está compilado)

```bash
rm -r CMakeCache.txt CMakeFiles/ cmake_install.cmake Makefile kim_aggarwal
cd build/ && cmake ..
make
```

---

