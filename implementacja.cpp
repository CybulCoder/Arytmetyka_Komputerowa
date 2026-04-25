#include<iostream>
#include<iomanip>
#include<vector>
#include<algorithm>
#include<cstdint>
#include<fstream>
#include<cmath>
#include<string>
#include<sstream>
#include<chrono>

//stałe do przełączania (liczba słów)
//zwykłe -> Karatsuba
int N0 = 40;
//Karatsuba -> ToomCook3
int N1 = 200;

struct WielkaLiczba {
    std::vector<uint32_t> slowa;
    bool czy_ujemna = false;

    //konstruktor domyslny
    WielkaLiczba() {};

    WielkaLiczba(std::vector<uint32_t> liczba) : slowa(std::move(liczba)) {}
};

// 1 gdy a wieksze, -1 gdy b wieksze 0 gdy rowne
int porownaj(const WielkaLiczba& a, const WielkaLiczba& b){
    if (a.slowa.size() > b.slowa.size()) return 1;
    if (a.slowa.size() < b.slowa.size()) return -1;

    for (int i = (int)a.slowa.size() - 1; i >= 0; i--){
        if (a.slowa[i] > b.slowa[i]) return 1;
        if (a.slowa[i] < b.slowa[i]) return -1;
    }
    return 0;
}

//znak roznicy a - b (wielkich liczb)
int znak_roznicy(const WielkaLiczba& a, const WielkaLiczba& b){
    if (a.slowa.size() > b.slowa.size()) return 1;
    if (a.slowa.size() < b.slowa.size()) return -1;

    for (int i = (int)a.slowa.size() - 1; i >= 0; i--){
        if (a.slowa[i] > b.slowa[i]) return 1;
        if (a.slowa[i] < b.slowa[i]) return -1;
    }
    return 1; //dla zera +
}

WielkaLiczba mnoz_skalar(const WielkaLiczba& a, uint32_t b){
    if (b == 0) return WielkaLiczba({0});
    std::vector<uint32_t> wynik;
    wynik.reserve(a.slowa.size() + 1);
    uint64_t carry = 0;

    for (size_t i = 0; i < a.slowa.size(); i++){
        uint64_t iloczyn = (uint64_t)a.slowa[i] * b + carry;
        wynik.push_back((uint32_t)(iloczyn & 0xFFFFFFFF));
        carry = iloczyn >> 32;
    }
    if (carry){
        wynik.push_back((uint32_t)carry);
    }
    return WielkaLiczba(wynik);
}

// wczytanie - trzeba najpierw otworzyc plik!!!
std::pair<WielkaLiczba, WielkaLiczba> wczytaj_dwie_liczby(std::ifstream& file){
    std::string line1, line2;
    if(std::getline(file, line1) && std::getline(file, line2)){
        std::vector<uint32_t> v1, v2;
        std::uint32_t word;
        std::stringstream ss1(line1);
        while (ss1 >> word){
            v1.push_back(word);
        }
        std::stringstream ss2(line2);
        while (ss2 >> word){
            v2.push_back(word);
        }
        
        // Zmiana na LSW na początku
        std::reverse(v1.begin(), v1.end());
        std::reverse(v2.begin(), v2.end());

        return {WielkaLiczba(v1), WielkaLiczba(v2)};
    } else {
        throw std::runtime_error("Wszystkie liczby z pliku zostały użyte");
    }
}

WielkaLiczba operator<<(const WielkaLiczba& a, int k)
{
    if (k == 0 || (a.slowa.size() == 1 && a.slowa[0] == 0)) return a;

    if (k % 32 != 0) {
        std::cout << "Ten operator obsluguje tylko wielokrotnosci 32\n";
        exit(1);
    }

    int word_shift = k / 32;
    
    // Z uwagi na little-endian doklejamy zera NA POCZĄTKU
    std::vector<uint32_t> wynik(word_shift, 0);
    wynik.insert(wynik.end(), a.slowa.begin(), a.slowa.end());

    return WielkaLiczba(wynik);
}

// dodawanie pisemne
WielkaLiczba dodawanie(const WielkaLiczba& a, const WielkaLiczba& b){
    std::vector<uint32_t> wynik;
    size_t n = std::max(a.slowa.size(), b.slowa.size());
    wynik.reserve(n + 1);
    uint64_t carry = 0;
    
    for (size_t i = 0; i < n; i++){
        uint64_t ai = (i < a.slowa.size()) ? a.slowa[i] : 0;
        uint64_t bj = (i < b.slowa.size()) ? b.slowa[i] : 0;

        uint64_t sum = ai + bj + carry;
        wynik.push_back((uint32_t)(sum & 0xFFFFFFFF));
        carry = sum >> 32;
    }
    if (carry){
        wynik.push_back(carry);
    }
    return WielkaLiczba(wynik);
}

// wyswietlanie
void print_words(const WielkaLiczba& a)
{
    std::cout << "[ ";
    // Dla człowieka drukujemy od MSW do LSW (od tyłu wektora)
    for (int i = (int)a.slowa.size() - 1; i >= 0; i--)
    {
        std::cout << a.slowa[i] << " ";
    }
    std::cout << "]\n";
}

// odejmowanie pisemne a - b: wieksza - mniejsza!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
WielkaLiczba odejmij(const WielkaLiczba& a, const WielkaLiczba& b){
    std::vector<uint32_t> wynik;
    wynik.reserve(a.slowa.size());
    uint64_t borrow = 0;

    for (size_t i = 0; i < a.slowa.size(); i++){
        uint64_t ai = a.slowa[i];
        uint64_t bi = (i < b.slowa.size()) ? b.slowa[i] : 0;

        uint64_t do_odjecia = bi + borrow;
        if (ai < do_odjecia){
            wynik.push_back((uint32_t)(ai + (1ULL << 32) - do_odjecia));
            borrow = 1;
        } else {
            wynik.push_back((uint32_t)(ai - do_odjecia));
            borrow = 0;
        }
    }
    // usuniecie wiodacych zer (teraz wiodące zera są na końcu wektora)
    while (wynik.size() > 1 && wynik.back() == 0){
        wynik.pop_back();
    }
    return WielkaLiczba(wynik);
}

//moduł roznicy A-B
WielkaLiczba modul_roz(const WielkaLiczba& a, const WielkaLiczba& b){
    int znak = znak_roznicy(a, b);
    if (znak == 1){
        return odejmij(a, b);
    } 
    return odejmij(b, a);
}

//modulo beta^k
WielkaLiczba mod_podst_k(const WielkaLiczba& a, int k){
    if (k >= (int)a.slowa.size()){
        return a;
    }
    std::vector<uint32_t> wynik(
        a.slowa.begin(),
        a.slowa.begin() + k
    );
    return WielkaLiczba(wynik);
}

// dzielenie całkowite wielkiej liczby przez potęgę 2^32
WielkaLiczba dziel_calk_pot(const WielkaLiczba& a, int k){
    if (k >= (int)a.slowa.size()){
        return WielkaLiczba({0});
    }
    std::vector<uint32_t> wynik(
        a.slowa.begin() + k,
        a.slowa.end()
    );
    return WielkaLiczba(wynik);
}

// Optyymalizacja mnozenia pisemnego - czysty algorytm O(N^2) bez niepotrzebnych alokacji wektorów w petli
WielkaLiczba mnoz_pisemne(const WielkaLiczba& a, const WielkaLiczba& b){
    if ((a.slowa.size() == 1 && a.slowa[0] == 0) || (b.slowa.size() == 1 && b.slowa[0] == 0)){
        return WielkaLiczba({0});
    }
    std::vector<uint32_t> wynik(a.slowa.size() + b.slowa.size(), 0);
    
    for (size_t i = 0; i < a.slowa.size(); i++) {
        uint64_t carry = 0;
        for (size_t j = 0; j < b.slowa.size(); j++) {
            uint64_t cur = wynik[i + j] + (uint64_t)a.slowa[i] * b.slowa[j] + carry;
            wynik[i + j] = (uint32_t)(cur & 0xFFFFFFFF);
            carry = cur >> 32;
        }
        wynik[i + b.slowa.size()] += carry;
    }
    
    while (wynik.size() > 1 && wynik.back() == 0){
        wynik.pop_back();
    }
    return WielkaLiczba(wynik);
}

WielkaLiczba div_exact(const WielkaLiczba& a, uint32_t d)
{
    std::vector<uint32_t> wynik(a.slowa.size());
    uint64_t rem = 0;

    for (int i = (int)a.slowa.size() - 1; i >= 0; i--)
    {
        uint64_t cur = (rem << 32) | a.slowa[i];
        uint32_t q = (uint32_t)(cur / d);
        rem = cur % d;
        wynik[i] = q;
    }

    if (rem != 0)
    {
        std::cout << "blad: dzielenie nie jest exact\n";
    }

    // usuwanie zer wiodących (na końcu wektora z powodu little-endian)
    while (wynik.size() > 1 && wynik.back() == 0){
        wynik.pop_back();
    }
    
    return WielkaLiczba(wynik);
}

// mnozenie karatsuby
WielkaLiczba mnoz_karatsuba(const WielkaLiczba& a, const WielkaLiczba& b){
    int n = std::max((int)a.slowa.size(), (int)b.slowa.size());
    if (n < N0){
        return mnoz_pisemne(a, b);
    }
    int k = std::ceil(n / 2.0);

    WielkaLiczba A0 = mod_podst_k(a, k);
    WielkaLiczba B0 = mod_podst_k(b, k);
    WielkaLiczba A1 = dziel_calk_pot(a, k);
    WielkaLiczba B1 = dziel_calk_pot(b, k);
    int s_a = znak_roznicy(A0, A1);
    int s_b = znak_roznicy(B0, B1);
    WielkaLiczba C0 = mnoz_karatsuba(A0, B0);
    WielkaLiczba C1 = mnoz_karatsuba(A1, B1);
    WielkaLiczba X = modul_roz(A0, A1);
    WielkaLiczba Y = modul_roz(B0, B1);
    WielkaLiczba C2 = mnoz_karatsuba(X, Y);
    
    WielkaLiczba wynik = C0;
    if (s_a * s_b == -1){
        C2 = dodawanie(C2, C1);
        C2 = dodawanie(C2, C0);
        C2 = C2 << (k * 32);
        C1 = C1 << (2 * k * 32);
    } else {
        WielkaLiczba wsp = dodawanie(C1, C0);
        C2 = odejmij(wsp, C2);
        C2 = C2 << (k * 32);
        C1 = C1 << (2 * k * 32);
    }
    wynik = dodawanie(wynik, C2);
    wynik = dodawanie(wynik, C1);
    return wynik;
}

//Toom-Cook-3
WielkaLiczba mnoz_toomcook3(const WielkaLiczba& a, const WielkaLiczba& b){
    if (((a.slowa.size() == 1) && (a.slowa[0] == 0)) || ((b.slowa.size() == 1) && (b.slowa[0] == 0))){
        return WielkaLiczba({0});
    }
    int n = std::max((int)a.slowa.size(), (int)b.slowa.size());
    if (n < N1){
        return mnoz_karatsuba(a, b);
    }
    int k = std::ceil(n / 3.0);
    
    WielkaLiczba a0 = mod_podst_k(a, k);
    WielkaLiczba a1 = dziel_calk_pot(a, k);
    a1 = mod_podst_k(a1, k);
    WielkaLiczba a2 = dziel_calk_pot(a, 2*k);
    
    WielkaLiczba b0 = mod_podst_k(b, k);
    WielkaLiczba b1 = dziel_calk_pot(b, k);
    b1 = mod_podst_k(b1, k);
    WielkaLiczba b2 = dziel_calk_pot(b, 2*k);

    WielkaLiczba v0 = mnoz_toomcook3(a0, b0);
    WielkaLiczba a02 = dodawanie(a0, a2);
    WielkaLiczba b02 = dodawanie(b0, b2);
    WielkaLiczba temp1 = dodawanie(a02, a1);
    WielkaLiczba temp2 = dodawanie(b02, b1);
    WielkaLiczba v1 = mnoz_toomcook3(temp1, temp2);

    int znak;
    znak = znak_roznicy(a02, a1);
    if (znak == 1){
        temp1 = odejmij(a02, a1);
    } else {
        temp1 = odejmij(a1, a02);
        temp1.czy_ujemna = true;
    }
    znak = znak_roznicy(b02, b1);
    if (znak == 1){
        temp2 = odejmij(b02, b1);
    } else {
        temp2 = odejmij(b1, b02);
        temp2.czy_ujemna = true;
    }
    
    WielkaLiczba v_1 = mnoz_toomcook3(temp1, temp2);
    if (temp1.czy_ujemna ^ temp2.czy_ujemna){
        v_1.czy_ujemna = true;
    }
    temp1.czy_ujemna = false;
    temp2.czy_ujemna = false;
    
    temp1 = mnoz_skalar(a2, 4);
    temp2 = mnoz_skalar(b2, 4);
    WielkaLiczba temp3 = mnoz_skalar(a1, 2);
    WielkaLiczba temp4 = mnoz_skalar(b1, 2);

    temp3 = dodawanie(temp3, temp1);
    temp4 = dodawanie(temp4, temp2);
    temp3 = dodawanie(temp3, a0);
    temp4 = dodawanie(temp4, b0);
    WielkaLiczba v2 = mnoz_toomcook3(temp3, temp4);

    WielkaLiczba v_n = mnoz_toomcook3(a2, b2);

    temp1 = mnoz_skalar(v0, 3);
    temp2 = mnoz_skalar(v_1, 2);
    
    WielkaLiczba t1;
    if (v_1.czy_ujemna){
        znak = znak_roznicy(temp1, temp2);
        if (znak == 1){
            t1 = odejmij(temp1, temp2);
        } else {
            t1 = odejmij(temp2, temp1);
            t1.czy_ujemna = true;
        }
    } else {
        t1 = dodawanie(temp1, temp2);
    }
    
    if (t1.czy_ujemna){
        znak = znak_roznicy(v2, t1);
        if (znak == 1){
            t1 = odejmij(v2, t1);
            t1.czy_ujemna = false;
        } else {
            t1 = odejmij(t1, v2);
            t1.czy_ujemna = true;
        }
    } else {
        t1 = dodawanie(t1, v2);
    }

    t1 = div_exact(t1, 6);
    temp3 = mnoz_skalar(v_n, 2);
    
    if (t1.czy_ujemna){
        t1 = dodawanie(t1, temp3);
    } else {
        znak = znak_roznicy(t1, temp3);
        if (znak == 1){
            t1 = odejmij(t1, temp3);
        } else {
            t1 = odejmij(temp3, t1);
            t1.czy_ujemna = true;
        }
    }
    
    WielkaLiczba t2;
    if (v_1.czy_ujemna){
        znak = znak_roznicy(v1, v_1);
        if (znak == 1){
            t2 = odejmij(v1, v_1);
        } else {
            t2 = odejmij(v_1, v1);
            t2.czy_ujemna = true;
        }
    } else {
        t2 = dodawanie(v1, v_1);
    }

    t2 = div_exact(t2, 2);

    WielkaLiczba c1;
    if (t1.czy_ujemna){
        c1 = dodawanie(v1, t1);
    } else {
        c1 = odejmij(v1, t1);
    }
    
    WielkaLiczba c2 = odejmij(t2, v0);
    c2 = odejmij(c2, v_n); 
    WielkaLiczba c3 = odejmij(t1, t2);

    c1 = c1 << (k * 32);
    c2 = c2 << (k * 2 * 32);
    c3 = c3 << (k * 3 * 32);
    v_n = v_n << (k * 4 * 32);

    v0 = dodawanie(v0, c1);
    v0 = dodawanie(v0, c2);
    v0 = dodawanie(v0, c3);
    v0 = dodawanie(v0, v_n);

    return v0;
}

struct TimeResult {
    double value;
    std::string unit;
};

TimeResult choose_time_unit(double input_ns) {
    TimeResult result;
    if (input_ns < 1000) {
        result.value = input_ns;
        result.unit = "ns";
    } else if (input_ns < 1'000'000) {
        result.value = input_ns / 1000;
        result.unit = "us";
    } else if (input_ns < 1'000'000'000) {
        result.value = input_ns / 1'000'000;
        result.unit = "ms";
    } else {
        result.value = input_ns / 1'000'000'000;
        result.unit = "s";
    }
    return result;
}

int main(){
    std::vector<size_t> sizes = {50, 100, 150, 200, 250, 300, 350, 400, 450, 500,
        550, 600, 650, 700, 750, 800, 850, 900, 950, 1000}; 
    std::string output_csv = "resultsToom.csv";
    std::ofstream file_csv(output_csv, std::ios::app);
    if (!file_csv){
        std::cout << "error opening resultsToom.csv file.\n";
        return 1;
    }
    
    for (size_t size : sizes){
        std::string input_filename = "Dane_wygenerowane/data_" + std::to_string(size) + ".txt";
        std::ifstream file(input_filename);
        if (!file){
            std::cout << "Error opening file with numbers\n";
            return 1;
        }
        std::string line1, line2;
        while(std::getline(file, line1) && std::getline(file, line2)){
            std::vector<uint32_t> vec1, vec2;
            uint32_t word;

            std::stringstream ss1(line1);
            while (ss1 >> word){
                vec1.push_back(word);
            }

            std::stringstream ss2(line2);
            while(ss2 >> word){
                vec2.push_back(word);
            }

            // Odwrócenie tak, żeby index 0 trzymał LSW
            std::reverse(vec1.begin(), vec1.end());
            std::reverse(vec2.begin(), vec2.end());

            size_t num_words = vec1.size();

            WielkaLiczba a(vec1);
            WielkaLiczba b(vec2);

            auto start = std::chrono::high_resolution_clock::now();
            WielkaLiczba wynik = mnoz_toomcook3(a, b);
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();

            TimeResult time_result = choose_time_unit(duration);
            std::cout << "Execution time for " << num_words << " words: " << time_result.value << " " << time_result.unit << std::endl;
            file_csv << num_words << "," << time_result.value << "," << time_result.unit << std::endl;

        }
        file.close();
    }
    file_csv.close();
    return 0;
}