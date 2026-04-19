//TOOM COOK 3
//v2

#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <sstream>
#include <string>


//Wielka liczba
struct WielkaLiczba
{
    std::vector<uint32_t> fragmenty;
    bool czy_ujemna = false;

    WielkaLiczba() {}

    WielkaLiczba(std::vector<uint32_t> vec, bool ujemna = false)
    {
        fragmenty = vec;
        czy_ujemna = ujemna;
        usun_zera();
    }

    void usun_zera()
    {
        while (fragmenty.size() > 1 && fragmenty.back() == 0)
        {
            fragmenty.pop_back();
        }
        if (fragmenty.empty())
        {
            fragmenty.push_back(0);
        }
        if (czy_zero())
        {
            czy_ujemna = false;
        }
    }
    bool czy_zero() const
    {
        return fragmenty.size() == 1 && fragmenty[0] == 0;
    }
};


//funkcje pomocnicze

//Dodawanie

WielkaLiczba dodaj(const WielkaLiczba& a, const WielkaLiczba& b)
{
    WielkaLiczba wynik;
    uint64_t przeniesienie = 0;
    size_t max_rozmiar = std::max(a.fragmenty.size(), b.fragmenty.size());

    for (size_t i = 0; i < max_rozmiar || przeniesienie > 0;i++)
    {
        uint64_t suma = przeniesienie;
        if (i < a.fragmenty.size())
        {
            suma += a.fragmenty[i];
        }
        if (i < b.fragmenty.size())
        {
            suma += b.fragmenty[i];
        }
        wynik.fragmenty.push_back(suma & 0xFFFFFFFF);
        przeniesienie = suma >> 32;
    }
    return wynik;
}

//odejmowanie (zakladamy ze a>=b)
WielkaLiczba odejmij(const WielkaLiczba& a, const WielkaLiczba& b)
{
    WielkaLiczba wynik;
    uint64_t pozycz = 0;

    for (size_t i = 0; i < a.fragmenty.size(); i++)
    {
        uint64_t liczba1 = a.fragmenty[i];
       
        uint64_t fragb;
        if (i < b.fragmenty.size())
        {
            fragb = b.fragmenty[i];
        }
        else
        {
            fragb = 0;
        }
        uint64_t liczba2 = fragb + pozycz;

        if (liczba1 < liczba2)
        {
            wynik.fragmenty.push_back((0x100000000ULL + liczba1 - liczba2) & 0xFFFFFFFF);
            pozycz = 1;
        }
        else
        {
            wynik.fragmenty.push_back((liczba1 - liczba2) & 0xFFFFFFFF);
            pozycz = 0;
        }
       
    }
    wynik.usun_zera();
    return wynik;
    
}

//przesuniecie 

WielkaLiczba przesun_w_lewo(WielkaLiczba a, size_t rozm)
{
    if (a.czy_zero() || rozm == 0)
    {
        return a;
    }
    a.fragmenty.insert(a.fragmenty.begin(), rozm, 0);
    return a;
}

//potrzebne do interpolacji
WielkaLiczba dzielenie_interp(const WielkaLiczba& a, uint32_t dzielnik)
{
    WielkaLiczba wynik;
    wynik.fragmenty.resize(a.fragmenty.size());
    uint64_t reszta = 0;
    for (int i = a.fragmenty.size() - 1; i >= 0; i--)
    {
        uint64_t aktualna = (reszta << 32) | a.fragmenty[i];
        wynik.fragmenty[i] = aktualna / dzielnik;
        reszta = aktualna % dzielnik;
    }
    wynik.usun_zera();
    return wynik;
}

//Mnozenie
WielkaLiczba Mnozenie(const WielkaLiczba& a, const WielkaLiczba& b)
{
    if (a.czy_zero() || b.czy_zero())
    {
        return WielkaLiczba({ 0 });
    }

    std::vector<uint32_t> wynik_bufor(a.fragmenty.size() + b.fragmenty.size(), 0);

    for (size_t i = 0; i < a.fragmenty.size(); i++)
    {
        uint32_t cyfra_a = a.fragmenty[i];
        uint64_t przeniesienie = 0;
        for (size_t j = 0; j < b.fragmenty.size();j++)
        {
            uint32_t cyfra_b = b.fragmenty[j];
            
            uint64_t iloczyn = static_cast<uint64_t> (cyfra_a) * (cyfra_b)+wynik_bufor[i + j] + przeniesienie;
            wynik_bufor[i + j] = static_cast<uint64_t> (iloczyn & 0xFFFFFFFF);
            przeniesienie = iloczyn >> 32;
        }
        if (przeniesienie > 0)
        {
            wynik_bufor[i + b.fragmenty.size()] += static_cast<uint32_t>(przeniesienie);

        }
    }
    WielkaLiczba wynik_koncowy(wynik_bufor);
    wynik_koncowy.czy_ujemna = (a.czy_ujemna != b.czy_ujemna);
    wynik_koncowy.usun_zera();
    return wynik_koncowy;
}


//TOOM COOK 3
WielkaLiczba Toom_Cook3(const WielkaLiczba& a, const WielkaLiczba& b)
{
    // 1. Zakończenie rekurencji: jeśli liczby są małe, używamy standardowego mnożenia
    size_t rozmiar = std::max(a.fragmenty.size(), b.fragmenty.size());
    if (rozmiar < 9) {
        return Mnozenie(a, b);
    }
 

    // 2. Splitting (Podział): dzielimy liczbę na 3 równe części
    size_t k = (rozmiar + 2) / 3;

    auto wytnij_fragment = [&](const WielkaLiczba& liczba, size_t start) {
        if (start >= liczba.fragmenty.size()) return WielkaLiczba({ 0 });
        size_t koniec = std::min(start + k, liczba.fragmenty.size());
        return WielkaLiczba(std::vector<uint32_t>(liczba.fragmenty.begin() + start, liczba.fragmenty.begin() + koniec));
    };

    // A(x) = a0 + a1*x + a2*x^2
    WielkaLiczba a0 = wytnij_fragment(a, 0);
    WielkaLiczba a1 = wytnij_fragment(a, k);
    WielkaLiczba a2 = wytnij_fragment(a, 2 * k);

    // B(x) = b0 + b1*x + b2*x^2
    WielkaLiczba b0 = wytnij_fragment(b, 0);
    WielkaLiczba b1 = wytnij_fragment(b, k);
    WielkaLiczba b2 = wytnij_fragment(b, 2 * k);

    // 3. Evaluation (Ewaluacja w 5 punktach: 0, 1, -1, -2, inf)
    WielkaLiczba p0 = a0;
    WielkaLiczba p1 = dodaj(dodaj(a0, a1), a2);
    WielkaLiczba p_m1 = dodaj(a0, a2);
    p_m1 = odejmij(p_m1, a1);
    WielkaLiczba p_m2 = dodaj(a0, przesun_w_lewo(a2, 2));
    p_m2 = odejmij(p_m2, przesun_w_lewo(a1, 1));
    WielkaLiczba p_inf = a2;

    WielkaLiczba q0 = b0;
    WielkaLiczba q1 = dodaj(dodaj(b0, b1), b2);
    WielkaLiczba q_m1 = odejmij(dodaj(b0, b2), b1);
    WielkaLiczba q_m2 = odejmij(dodaj(b0, przesun_w_lewo(b2, 2)), przesun_w_lewo(b1, 1));
    WielkaLiczba q_inf = b2;

    // 4. Pointwise Multiplication (Rekurencyjne mnożenie)
    WielkaLiczba r0 = Toom_Cook3(p0, q0);
    WielkaLiczba r1 = Toom_Cook3(p1, q1);
    WielkaLiczba r_m1 = Toom_Cook3(p_m1, q_m1);
    WielkaLiczba r_m2 = Toom_Cook3(p_m2, q_m2);
    WielkaLiczba r_inf = Toom_Cook3(p_inf, q_inf);

    // 5. Interpolation (Odwracanie macierzy - Sekwencja Bodrato)
    WielkaLiczba v0 = r0;
    WielkaLiczba v4 = r_inf;

    WielkaLiczba v3 = dzielenie_interp(odejmij(r_m2, r1), 3);
    WielkaLiczba v1 = dzielenie_interp(odejmij(r1, r_m1), 2);
    WielkaLiczba v2 = odejmij(r_m1, r0);

    v3 = dodaj(dzielenie_interp(odejmij(v2, v3), 2), przesun_w_lewo(r_inf, 1));
    v2 = odejmij(dodaj(v2, v1), r_inf);
    v1 = odejmij(v1, v3);

    // 6. Recomposition (Składanie wyniku)
    // Wynik = v0 + v1*B^k + v2*B^(2k) + v3*B^(3k) + v4*B^(4k)
    WielkaLiczba wynik = v0;
    wynik = dodaj(wynik, przesun_w_lewo(v1, k));
    wynik = dodaj(wynik, przesun_w_lewo(v2, 2 * k));
    wynik = dodaj(wynik, przesun_w_lewo(v3, 3 * k));
    wynik = dodaj(wynik, przesun_w_lewo(v4, 4 * k));

    return wynik;


}


void wypisz_liczbe(const std::string& etykieta, const WielkaLiczba& liczba) {
    std::cout << etykieta << ": ";
    if (liczba.czy_ujemna) std::cout << "-";
    std::cout << "0x";
    for (int i = liczba.fragmenty.size() - 1; i >= 0; i--) {
        std::cout << std::setfill('0') << std::setw(8) << std::hex << liczba.fragmenty[i];
    }
    std::cout << std::dec << "\n";
}



int main()
{
    WielkaLiczba a1({ 0x00000005 }); // 5
    WielkaLiczba b1({ 0x0000000A }); // 10

    
    wypisz_liczbe("Liczba A", a1);
    wypisz_liczbe("Liczba B", b1);
    wypisz_liczbe("Mnożenie ToomCook", Toom_Cook3(a1, b1));
    std::cout << "\n";

    return 0;
}


