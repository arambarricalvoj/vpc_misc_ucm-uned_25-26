import cv2
import numpy as np

points = []

def click_event(event, x, y, flags, param):
    global points

    # Clic izquierdo -> añadir punto
    if event == cv2.EVENT_LBUTTONDOWN:
        points.append((x, y))
        print("Punto añadido:", (x, y))

    # Clic derecho -> eliminar último punto
    if event == cv2.EVENT_RBUTTONDOWN:
        if len(points) > 0:
            removed = points.pop()
            print("Punto eliminado:", removed)
        else:
            print("No hay puntos para eliminar")

# Cargar imagen base (primer frame)
img = cv2.imread("M-30-HD/image000001.jpg")
clone = img.copy()

cv2.namedWindow("Selecciona ROI")
cv2.setMouseCallback("Selecciona ROI", click_event)

while True:
    temp = clone.copy()

    # Dibujar puntos
    for p in points:
        cv2.circle(temp, p, 4, (0, 0, 255), -1)

    # Dibujar líneas entre puntos
    if len(points) > 1:
        cv2.polylines(temp, [np.array(points)], False, (0, 255, 0), 2)

    cv2.imshow("Selecciona ROI", temp)

    key = cv2.waitKey(1) & 0xFF

    if key == 13:  # ENTER para terminar
        break
    if key == 27:  # ESC para cancelar
        points = []
        break

cv2.destroyAllWindows()

# Crear máscara
mask = np.zeros(img.shape[:2], dtype=np.uint8)

if len(points) > 2:
    cv2.fillPoly(mask, [np.array(points)], 255)

# Guardar máscara
np.save("roi.npy", mask)
print("Máscara guardada en roi.npy")
