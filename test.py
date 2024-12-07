import numpy as np

def winkel_von_2_vektoren(a, b):
    # Umwandlung der Vektoren in numpy-Arrays
    a = np.array(a)
    b = np.array(b)

    # Berechnung des Skalarprodukts (inneres Produkt)
    skalarprodukt = np.dot(a, b)

    # Berechnung der Längen der Vektoren
    laenge_a = np.linalg.norm(a)
    laenge_b = np.linalg.norm(b)

    # Berechnung des Winkels in Bogenmaß
    cos_theta = skalarprodukt / (laenge_a * laenge_b)
    winkel_rad = np.arccos(np.clip(cos_theta, -1.0, 1.0))  # Clip für numerische Stabilität
    winkel_deg = np.degrees(winkel_rad)  # Umrechnung in Grad

    # Berechnung des Kreuzprodukts
    kreuzprodukt = np.cross(a, b)

    # Berechnung des Vorzeichens der z-Komponente des Kreuzprodukts
    # Der Vorzeichen des Kreuzprodukts entscheidet, ob der Winkel mehr als 180° ist
    # Wenn das Kreuzprodukt einen positiven z-Wert hat, ist der Winkel < 180°, andernfalls > 180°
    if kreuzprodukt[2] < 0:  # Wenn die z-Komponente negativ ist, dann ist der Winkel > 180°
        winkel_deg = 360 - winkel_deg  # Wir subtrahieren den Winkel von 360°, um ihn korrekt darzustellen

    return winkel_deg

# Beispiel: Vektoren a und b (z. B. die Ausrichtung der Beschleunigungssensoren)
a = [1, 0, 0]  # Beispiel-Vektor 1
b = [0, 0, -1]  # Beispiel-Vektor 2

# Berechnung des Winkels
winkel = winkel_von_2_vektoren(a, b)
print(f"Der Winkel zwischen den Vektoren beträgt {winkel} Grad.")
