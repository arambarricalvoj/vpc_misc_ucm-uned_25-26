# Tema 3: Correcciones radiométricas y geométricas. Color
Implementaciones del tema 3.
## Ejecución

```bash
cd build/ && ./ecualizacion input.png output.png metodo [param]
cd build/ && ./filtroGaussiano input_image output_image [tamaño_kernel] [sigma]
```

Por ejemplo:
```bash
cd build/ && ./ecualizacion bajoContraste1.png bajoConstraste1_out.png exponencial 0.5
cd build/ && ./filtroGaussiano lena.png lena_out.png 7 1.5
```

## Compilación (ya está compilado)

```bash
rm -r CMakeCache.txt CMakeFiles/ cmake_install.cmake Makefile ecualizacion filtroGaussiano
cd build/ && cmake ..
make
```

---