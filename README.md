# 💾 Implementacja algorytmu mnożenia Toom-Cook'a (Toom-3) dla liczb całkowitych dowolnej precyzji w C++. Zaprojektowanie i przeprowadzenie eksperymentu mającego na celu porównanie utworzonej implementacji z istniejącymi bibliotekami: GMP i wbudowaną biblioteką w Python. 

# UWAGA: Projekt w trakcie realizacji! 🚧 🏗️ 🚧

### 📂 `Dane_wygenerowane`

Opis:

> Folder zawiera dane wygenerowane przez data_gen.py

Możesz tu napisać:

* Pliki tekstowe: data_50.txt, data_100.txt, ..., data_950.txt, data_1000.txt
* W każdej linii jedna liczba o bazie 2^32
* Słowa maszynowe oddzielone znakiem spacji
* Kolejność od ,,cyfry" najbardziej znaczącej, do najmniej

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

* Należy zmienić zmienną SIZE, by wykonać test dla danej próbki
* Zapisuje wyniki do pliku 📊**resultsPYTHON.csv** w formacie: SIZE, time
* time jest podany w mikrosekundach

---

### ⚙️ `main_GMP.cpp`

Opis:

> Mierzy czas wykonania mnożenia przez bibliotekę GNU MP.


* Należy zmienić zmienną SIZE, by wykonać test dla danej próbki
* Zapisuje wyniki do pliku 📊**resultsGMP.csv** w formacie: SIZE, time
* time jest podany w mikrosekundach


Przykład kompilacji:
```bash
g++ main_GMP.cpp -o program -lgmp
```
