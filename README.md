# 🌐 Simulazione telo esastico (FF7 Style)

Simulazione in C++17 con titolo all'apertura **"Simulazione telo esastico"**, telo elastico deformabile attorno alle sfere, inclinazione della gravità con **WASD / Frecce**, telecamera libera e interfaccia in stile **Final Fantasy VII**.

---

## 📂 Struttura del Progetto (Architettura MVC)

* **`src/model/` (Logica e Fisica)**: Gestisce i nodi del telo, la fisica massa-molla, le sfere, la gravità direzionale (`gravityVec` inclinata da WASD / Frecce), le collisioni e l'elasticità (`elasticity`).
* **`src/view/` (Grafica e UI)**: Proietta la scena con telecamera libera (rotazione, inclinazione, zoom, traslazione) e disegna le finestre FF7 con spaziature sicure.
* **`src/controller/` (Input e Flusso)**: Gestisce input da tastiera e mouse (inclinazione gravità con WASD/Frecce, rotazione camera con Q/E, trascinamento/zoom, slider elasticità, spawn sfere, help, pausa, reset, griglia).
* **`src/main.cpp`**: Avvia la finestra `"Simulazione telo esastico"` e il ciclo principale.

---

## 🎮 Comandi Disponibili

* **Tasti `W` / `A` / `S` / `D` o `Frecce`**: Inclinano la gravità lungo il piano del telo, facendo scivolare/rotolare le sfere in quella direzione.
* **Tasto `F`** (oppure **`INVIO`**): Fa cadere una nuova sfera sopra il telo.
* **Click Sinistro (tenuto premuto + trascina)**: Sposta la visuale sullo schermo.
* **Tasti `Q` / `E`**: Ruota la telecamera a sinistra / destra.
* **Rotella Mouse**: Zoom avanti / indietro.
* **Barra Elasticità**: Centro (`0.50`) = default; a destra (`1.00`) = più cedevole, la sfera affonda di più.
* **Tasto `H`**: Mostra / nasconde la finestra Help.
* **Tasto `SPAZIO`**: Pausa / riprende la simulazione.
* **Tasto `R`**: Resetta telo, sfere, gravità e telecamera.
* **Tasto `G`**: Mostra / nasconde la griglia del telo.
