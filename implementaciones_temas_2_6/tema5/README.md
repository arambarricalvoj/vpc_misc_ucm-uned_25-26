# Tema 5: Descripción de bordes y regiones
Implementaciones del tema 5.
## Ejecución

```bash
cd build/ && ./hough input_image output_image [sigma] [filtro-laplaciano] [kRel] [modo = "lines"] [thetaBins] [rhoStep] [lineVotes] [lineNMS] [maxLines]
cd build/ && ./hough input_image output_image [sigma] [filtro-laplaciano] [kRel] [modo = "circles"] [rMin] [rMax] [rStep] [angleBins] [circleVotes] [circleNMS] [maxCircles]
cd build/ && ./snakes rojo_186.png snakes_186_120p.png [N] [sigma] [alpha] [beta] [gamma] [win] [T1] [T2] [T3] [maxIter]
```

Por ejemplo:
```bash
cd build/ && ./hough original.png out_lines.png 1.0 8 0.5 lines --thetaBins 180 --rhoStep 1 --lineVotes 80 --maxLines 10
cd build/ && ./hough circ.jpg circ_output.png 1.0 8 0 circles   --rMin 10 --rMax 80 --rStep 2   --angleBins 360 --circleVotes 60 --circleNMS 2 --maxCircles 8
cd build/ && ./snakes rojo_186.png snakes_186_120p.png 200 1.0 0.8 0.6 5.0 7 3 0.02 20 200
```

## Compilación (ya está compilado)

```bash
rm -r CMakeCache.txt CMakeFiles/ cmake_install.cmake Makefile hough snakes
cd build/ && cmake ..
make
```

---