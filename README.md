# 💾 Implementacja algorytmu mnożenia Toom-Cook'a (Toom-3) dla liczb całkowitych dowolnej precyzji w C++. Zaprojektowanie i przeprowadzenie eksperymentu mającego na celu porównanie utworzonej implementacji z istniejącymi bibliotekami: GMP i wbudowaną biblioteką w Python. 

# UWAGA: Projekt w trakcie realizacji! 🚧 🏗️ 🚧

### 📂 `Dane_wygenerowane`

Opis:

> Folder zawiera dane wygenerowane przez data_gen.py

* Pliki tekstowe: data_50.txt, data_100.txt, ..., data_950.txt, data_1000.txt
* W każdej linii jedna liczba o bazie 2^32
* Słowa maszynowe oddzielone znakiem spacji
* Kolejność od ,,cyfry" najbardziej znaczącej, do najmniej znaczącej

---

### 📂 `Dokumentacja`

Opis:

> Dokumentacja techniczna projektu, kod w .tex

Zawiera
* Dokumentacja.pdf
* main.tex -- kod główny
* references.bib -- elementy bibliografii
* Oraz zdjęcia wykorzystane

---

### 📂 `Notki_z_spotkan`

Opis:

> Notatki z spotkań z opiekunem projektu

---

## 📄 Pliki

### 🐍 `data_gen.py`

Opis:

> Skrypt generujący dane wejściowe do testów.

* Nie przyjmuje, żadnych argumentów
* Korzysta z modułu random
* Generuje ,,cyfry" z rozkładem jednostajnym
* Tworzy pliki tekstowe: data_50.txt ...

---

### 🐍 `main_PYTHON.py`

Opis:

> Mierzy czas wykonania mnożenia przez wbudowany typ Python'a

* Pomiary przeprowadzone są dla każdej z długości w zmiennej sizes
* Zapisuje wyniki do pliku 📊**resultsPYTHON.csv** w formacie: SIZE, time, unit
* time może być podany w nanosekundach, mikrosekundach albo milisekundach, w zależności od wyniku 

---

### ⚙️ `main_GMP.cpp`

Opis:

> Mierzy czas wykonania mnożenia przez bibliotekę GNU MP.


* Pomiary przeprowadzone są dla każdej z długości w zmiennej sizes
* Zapisuje wyniki do pliku 📊**resultsGMP.csv** w formacie: SIZE, time, unit
* time może być podany w nanosekundach, mikrosekundach albo milisekundach, w zależności od wyniku

  Przykład kompilacji:
```bash
g++ main_GMP.cpp -o program -lgmp
```

---

### `analysis.py`

Opis:

> Pobiera dane z plików .csv, rysuje wykresy, dopasowuje regresję nieliniową

* Odczytanie plików .csv
* Wyrównanie jednostek
* Odrzucenie wartości odstających (reguła 3 sigma)
* Obliczenie średnich czasów i odchyleń standardowych
* Rysowanie wykresów
* Dopasowanie regresjii nieliniowej, model to:
   $$t = a \cdot x^n$$

  ---

  ### CMakeLists.txt

> Plik konfiguracyjny
  
*W katalogu roboczym należy utworzyć folder *build* i do niego przejść:
```bash
mkdir build
cd build
```
* Następnie należy wykonać polecenia:
```bash
cmake -G "MinGW Makefiles" ..
cmake --build . --target run_all
```
---
### requirements.txt

> Lista potrzebnych modułów pythona wykorzystanych w projekcie
