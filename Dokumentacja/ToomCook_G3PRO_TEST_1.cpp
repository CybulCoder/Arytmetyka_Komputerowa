#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>

// Nasza struktura do przechowywania bardzo dużych liczb.
// Liczba jest podzielona na "fragmenty" (każdy to 32-bitowa liczba całkowita).
struct WielkaLiczba {
    std::vector<uint32_t> fragmenty;
    bool czy_ujemna = false;

    // Konstruktor domyślny
    WielkaLiczba() {}

    // Konstruktor tworzący liczbę z wektora fragmentów
    WielkaLiczba(std::vector<uint32_t> f) {
        fragmenty = f;
        usun_wiodace_zera();
    }

    // Funkcja usuwająca niepotrzebne zera z końca wektora 
    // (np. 005 staje się 5. Zauważ, że wektor jest odwrócony, więc zera są na końcu).
    void usun_wiodace_zera() {
        while (fragmenty.size() > 1 && fragmenty.back() == 0) {
            fragmenty.pop_back();
        }
        if (fragmenty.empty()) {
            fragmenty.push_back(0);
        }
        if (fragmenty.size() == 1 && fragmenty[0] == 0) {
            czy_ujemna = false; // Zero nie jest ujemne
        }
    }

    bool czy_zero() const {
        return fragmenty.size() == 1 && fragmenty[0] == 0;
    }
};

// =====================================================================
// PODSTAWOWE OPERACJE MATEMATYCZNE (POMOCNICZE)
// =====================================================================

// Porównuje dwie liczby bez patrzenia na znak (tylko wielkość)
// Zwraca: 1 jeśli a > b, -1 jeśli a < b, 0 jeśli są równe
int porownaj_wielkosc(const WielkaLiczba& a, const WielkaLiczba& b) {
    if (a.fragmenty.size() > b.fragmenty.size()) return 1;
    if (a.fragmenty.size() < b.fragmenty.size()) return -1;

    // Zaczynamy od końca, bo tam są najważniejsze (najstarsze) cyfry
    for (int i = a.fragmenty.size() - 1; i >= 0; i--) {
        if (a.fragmenty[i] > b.fragmenty[i]) return 1;
        if (a.fragmenty[i] < b.fragmenty[i]) return -1;
    }
    return 0;
}

// Proste dodawanie pod kreską
WielkaLiczba dodaj_wartosci(const WielkaLiczba& a, const WielkaLiczba& b) {
    WielkaLiczba wynik;
    uint64_t przeniesienie = 0;
    size_t najwiekszy_rozmiar = std::max(a.fragmenty.size(), b.fragmenty.size());

    for (size_t i = 0; i < najwiekszy_rozmiar || przeniesienie > 0; i++) {
        uint64_t suma = przeniesienie;
        if (i < a.fragmenty.size()) suma += a.fragmenty[i];
        if (i < b.fragmenty.size()) suma += b.fragmenty[i];

        // Zapisujemy tylko 32 bity (resztę ucinamy)
        wynik.fragmenty.push_back(suma & 0xFFFFFFFF);
        // Reszta z dzielenia idzie dalej jako przeniesienie
        przeniesienie = suma >> 32;
    }
    return wynik;
}

// Proste odejmowanie pod kreską (zakładamy, że a >= b)
WielkaLiczba odejmij_wartosci(const WielkaLiczba& a, const WielkaLiczba& b) {
    WielkaLiczba wynik;
    uint64_t zapozyczenie = 0;

    for (size_t i = 0; i < a.fragmenty.size(); i++) {
        uint64_t odjemnik = zapozyczenie;
        if (i < b.fragmenty.size()) odjemnik += b.fragmenty[i];

        if (a.fragmenty[i] < odjemnik) {
            // Musimy pożyczyć z następnej cyfry
            uint64_t tymczasowa = (uint64_t)a.fragmenty[i] + 0x100000000ULL;
            wynik.fragmenty.push_back(tymczasowa - odjemnik);
            zapozyczenie = 1;
        }
        else {
            wynik.fragmenty.push_back(a.fragmenty[i] - odjemnik);
            zapozyczenie = 0;
        }
    }
    wynik.usun_wiodace_zera();
    return wynik;
}

// Inteligentne dodawanie i odejmowanie (obsługuje znaki + i -)
WielkaLiczba odejmij(WielkaLiczba a, WielkaLiczba b); // Deklaracja wyprzedzająca

WielkaLiczba dodaj(WielkaLiczba a, WielkaLiczba b) {
    if (a.czy_ujemna == b.czy_ujemna) {
        WielkaLiczba wynik = dodaj_wartosci(a, b);
        wynik.czy_ujemna = a.czy_ujemna;
        return wynik;
    }
    // Jeśli znaki są różne, zamieniamy dodawanie na odejmowanie
    if (porownaj_wielkosc(a, b) >= 0) {
        WielkaLiczba wynik = odejmij_wartosci(a, b);
        wynik.czy_ujemna = a.czy_ujemna;
        return wynik;
    }
    else {
        WielkaLiczba wynik = odejmij_wartosci(b, a);
        wynik.czy_ujemna = b.czy_ujemna;
        return wynik;
    }
}

WielkaLiczba odejmij(WielkaLiczba a, WielkaLiczba b) {
    b.czy_ujemna = !b.czy_ujemna; // Odwracamy znak b
    return dodaj(a, b); // Odejmowanie to dodawanie liczby przeciwnej
}

// Dzielenie przez stałą (potrzebne w algorytmie Toom-3)
WielkaLiczba podziel_przez_liczbe(WielkaLiczba a, uint32_t dzielnik) {
    WielkaLiczba wynik = a;
    uint64_t reszta = 0;
    // Przy dzieleniu idziemy od lewej do prawej (od końca wektora)
    for (int i = wynik.fragmenty.size() - 1; i >= 0; i--) {
        uint64_t aktualna_wartosc = (reszta << 32) + wynik.fragmenty[i];
        wynik.fragmenty[i] = aktualna_wartosc / dzielnik;
        reszta = aktualna_wartosc % dzielnik;
    }
    wynik.usun_wiodace_zera();
    return wynik;
}

// Przesunięcie liczby (mnożenie przez podstawę, dodanie zer na początek)
WielkaLiczba przesun(WielkaLiczba a, size_t ile_pozycji) {
    if (a.czy_zero()) return a;
    a.fragmenty.insert(a.fragmenty.begin(), ile_pozycji, 0);
    return a;
}

// Klasyczne mnożenie "pod kreską" dla małych liczb
WielkaLiczba mnozenie_szkolne(const WielkaLiczba& a, const WielkaLiczba& b) {
    if (a.czy_zero() || b.czy_zero()) return WielkaLiczba({ 0 });

    WielkaLiczba wynik;
    wynik.fragmenty.resize(a.fragmenty.size() + b.fragmenty.size(), 0);

    for (size_t i = 0; i < a.fragmenty.size(); i++) {
        uint64_t przeniesienie = 0;
        for (size_t j = 0; j < b.fragmenty.size(); j++) {
            uint64_t iloczyn = (uint64_t)a.fragmenty[i] * b.fragmenty[j] + wynik.fragmenty[i + j] + przeniesienie;
            wynik.fragmenty[i + j] = iloczyn & 0xFFFFFFFF;
            przeniesienie = iloczyn >> 32;
        }
        wynik.fragmenty[i + b.fragmenty.size()] = przeniesienie;
    }
    wynik.usun_wiodace_zera();
    wynik.czy_ujemna = (a.czy_ujemna != b.czy_ujemna);
    return wynik;
}

// =====================================================================
// GŁÓWNY ALGORYTM TOOM-COOK-3
// =====================================================================

WielkaLiczba toom3_mnozenie(WielkaLiczba u, WielkaLiczba v) {
    // KROK 0: Warunek stopu. 
    // Jeśli liczby są małe (mniej niż 5 fragmentów), zwykłe mnożenie jest szybsze.
    if (u.fragmenty.size() < 5 || v.fragmenty.size() < 5) {
        return mnozenie_szkolne(u, v);
    }

    // Obliczamy rozmiar, na jaki podzielimy liczby (na 3 równe części)
    size_t najdluzsza = std::max(u.fragmenty.size(), v.fragmenty.size());
    size_t k = (najdluzsza + 2) / 3;

    // Funkcja pomocnicza: dzieli wektor na 3 kawałki
    auto podziel = [k](const WielkaLiczba& liczba, WielkaLiczba& czesc0, WielkaLiczba& czesc1, WielkaLiczba& czesc2) {
        auto wytnij = [&](size_t start) {
            if (start >= liczba.fragmenty.size()) return WielkaLiczba({ 0 });
            size_t koniec = std::min(start + k, liczba.fragmenty.size());
            return WielkaLiczba(std::vector<uint32_t>(liczba.fragmenty.begin() + start, liczba.fragmenty.begin() + koniec));
        };
        czesc0 = wytnij(0);
        czesc1 = wytnij(k);
        czesc2 = wytnij(2 * k);
    };

    // KROK 1: Podział liczb U i V na 3 części
    WielkaLiczba u0, u1, u2, v0, v1, v2;
    podziel(u, u0, u1, u2);
    podziel(v, v0, v1, v2);

    // Funkcja pomocnicza: Oblicza wartości wielomianu w 5 punktach (0, 1, -1, -2, nieskończoność)
    auto ewaluacja = [](WielkaLiczba x0, WielkaLiczba x1, WielkaLiczba x2,
        WielkaLiczba& p0, WielkaLiczba& p1, WielkaLiczba& pm1, WielkaLiczba& pm2, WielkaLiczba& pinf) {
            p0 = x0;   // Wartość w punkcie 0
            pinf = x2; // Wartość w nieskończoności

            WielkaLiczba x0_plus_x2 = dodaj(x0, x2);
            p1 = dodaj(x0_plus_x2, x1);  // Wartość w punkcie 1
            pm1 = odejmij(x0_plus_x2, x1); // Wartość w punkcie -1

            // Wartość w punkcie -2: p(-2) = x0 - 2*x1 + 4*x2
            WielkaLiczba x1_razy_2 = dodaj(x1, x1);
            WielkaLiczba x2_razy_4 = dodaj(dodaj(x2, x2), dodaj(x2, x2));
            pm2 = odejmij(dodaj(x0, x2_razy_4), x1_razy_2);
    };

    // KROK 2: Ewaluacja
    WielkaLiczba pu0, pu1, pum1, pum2, puinf;
    WielkaLiczba pv0, pv1, pvm1, pvm2, pvinf;
    ewaluacja(u0, u1, u2, pu0, pu1, pum1, pum2, puinf);
    ewaluacja(v0, v1, v2, pv0, pv1, pvm1, pvm2, pvinf);

    // KROK 3: Rekurencyjne mnożenie w 5 punktach
    WielkaLiczba w0 = toom3_mnozenie(pu0, pv0);
    WielkaLiczba w1 = toom3_mnozenie(pu1, pv1);
    WielkaLiczba wm1 = toom3_mnozenie(pum1, pvm1);
    WielkaLiczba wm2 = toom3_mnozenie(pum2, pvm2);
    WielkaLiczba winf = toom3_mnozenie(puinf, pvinf);

    // KROK 4: Interpolacja (Odzyskiwanie współczynników wyniku - algorytm Bodrato)
    WielkaLiczba t1 = podziel_przez_liczbe(odejmij(wm2, w1), 3);
    WielkaLiczba t2 = podziel_przez_liczbe(odejmij(w1, wm1), 2);
    WielkaLiczba t3 = odejmij(wm1, w0);

    WielkaLiczba winf_razy_2 = dodaj(winf, winf);

    // Obliczamy ostateczne wartości współczynników (r0 do r4)
    WielkaLiczba r0 = w0;
    WielkaLiczba r4 = winf;
    WielkaLiczba r3 = dodaj(podziel_przez_liczbe(odejmij(t3, t1), 2), winf_razy_2);
    WielkaLiczba r2 = odejmij(dodaj(t3, t2), winf);
    WielkaLiczba r1 = odejmij(t2, r3);

    // KROK 5: Składanie wyniku w jedną wielką liczbę
    WielkaLiczba wynik = r0;
    wynik = dodaj(wynik, przesun(r1, k));
    wynik = dodaj(wynik, przesun(r2, 2 * k));
    wynik = dodaj(wynik, przesun(r3, 3 * k));
    wynik = dodaj(wynik, przesun(r4, 4 * k));

    // Ustawienie ostatecznego znaku
    wynik.czy_ujemna = (u.czy_ujemna != v.czy_ujemna);
    if (wynik.czy_zero()) wynik.czy_ujemna = false;

    return wynik;
}

// Funkcja drukująca liczby w formacie Hex (aby łatwo było sprawdzić 32-bitowe bloki)
void wypisz_liczbe(const WielkaLiczba& liczba) {
    if (liczba.czy_ujemna) std::cout << "-";
    std::cout << "0x";
    for (int i = liczba.fragmenty.size() - 1; i >= 0; i--) {
        std::cout << std::setfill('0') << std::setw(8) << std::hex << liczba.fragmenty[i];
    }
    std::cout << std::dec << "\n";
}

// =====================================================================
// GŁÓWNY PROGRAM
// =====================================================================
int main() {
    // 1. Przygotowanie dwóch liczb testowych
    WielkaLiczba liczba_A(std::vector<uint32_t>{
        0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666
    });

    WielkaLiczba liczba_B(std::vector<uint32_t>{
        0xAAAAAAAB, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD, 0xEEEEEEEE, 0xFFFFFFFF
    });

    std::cout << "--- Algorytm Mnozenia Toom-Cook-3 ---\n\n";
    std::cout << "Liczba A: "; wypisz_liczbe(liczba_A);
    std::cout << "Liczba B: "; wypisz_liczbe(liczba_B);

    // 2. Przemnożenie liczb
    WielkaLiczba wynik_toom = toom3_mnozenie(liczba_A, liczba_B);

    // 3. Wypisanie wyniku w konsoli
    std::cout << "\nWynik mnozenia (Toom-3): \n";
    wypisz_liczbe(wynik_toom);

    return 0;
}