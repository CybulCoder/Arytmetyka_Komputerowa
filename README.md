# 💾 Implementacja algorytmu mnożenia Toom-Cook'a (Toom-3) dla liczb całkowitych dowolnej precyzji w C++. Zaprojektowanie i przeprowadzenie eksperymentu mającego na celu porównanie utworzonej implementacji z istniejącymi bibliotekami: GMP i wbudowaną biblioteką w Python. 

# UWAGA: Projekt w trakcie realizacji! 🚧 🏗️ 🚧

---

### 📋 `Istrukcja uruchomienia pełnej analizy.pdf`

* Zawiera instrukcje do uruchomienia experymentu na własnym komputerze
* Należy się upewnić czy zainstalowane są odpowiednie pakiety podane w instrukcji

---

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

> Dokumentacja projektu


---

### 📂 `Notki_z_spotkan`

Opis:

> Notatki z spotkań z opiekunem projektu

---

### `CMakeLists.txt`

Opis:

> Plik konfiguracyjny

---

### `EXTRA_data_gen.py`

> Skrypt pomocniczy do wygenrowania danych do dodatkowego celu 1.

* Importuje funkcję Generate_Random_Number z skryptu data_gen.py
* Dane zostają zapisane w pliku o nazwie: 'EXTRA_data.txt'

---

### `EXTRAresultsGMP.csv`

> Wyniki czasowe mnożenia liczby z EXTRA_data.txt za pomocą GMP

---


### `STAN_PRAC_01_04_2026.txt`
> Plik informujący o stanie prac na dzień 01.04.2024
> Wymagane dla komisji projektowej

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

### 🐍 `data_gen.py`

Opis:

> Skrypt generujący dane wejściowe do testów.

* Nie przyjmuje, żadnych argumentów
* Korzysta z modułu random
* Generuje ,,cyfry" z rozkładem jednostajnym
* Tworzy pliki tekstowe: data_50.txt ...

---

### `fast_tests.cpp`

Opis:

> Plik źródłowy, za którego pomocą można sprawdzić czas wykonania algorytmów
> w zależności od przyjętego formatu

---

### `implementacja.cpp`

Opis:
> Plik źródłowy zawierający strukturę BigInt i algorytm mnożenia
> wygenerowany przez AI

---

### `implementacja_wersja_big_endian.cpp`

Opis:
> Druga wersja naszej implementacji, nieużywana w eksperymencie,
> korzystająca z formatu big endian

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

### 🐍 `main_PYTHON.py`

Opis:

> Mierzy czas wykonania mnożenia przez wbudowany typ Python'a

* Pomiary przeprowadzone są dla każdej z długości w zmiennej sizes
* Zapisuje wyniki do pliku 📊**resultsPYTHON.csv** w formacie: SIZE, time, unit
* time może być podany w nanosekundach, mikrosekundach albo milisekundach, w zależności od wyniku 

---

### `requirements.txt`

Opis:

> Lista potrzebnych modułów dla Pythona

---

### `resultsXXX.csv`

Opis:

> Wyniki czasowe dla danej biblioteki


