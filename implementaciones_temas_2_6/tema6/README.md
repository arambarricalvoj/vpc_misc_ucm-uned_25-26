# Tema 6: Operaciones morfológicas
Implementaciones del tema 6.
## Ejecución

```bash
cd build/ && ./opMorf input.png output.png [operacion] [tamanoEE] [tipoEE] [umbral_binario]
```

Por ejemplo:
```bash
cd build/ && ./opMorf input.jpg salida_erosion_17_cruz.jpg erosion 17 cruz
```

## Compilación (ya está compilado)

```bash
rm -r CMakeCache.txt CMakeFiles/ cmake_install.cmake Makefile opMorf
cd build/ && cmake ..
make
```

---