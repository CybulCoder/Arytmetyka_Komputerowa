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

//usuwac wiodacych zer nie trzeba bo takie dane nie bylyby wygenerowane

struct WielkaLiczba {
    std::vector<uint32_t> slowa;
    bool czy_ujemna = false;

    //konstruktor domyslny
    WielkaLiczba() {};

    WielkaLiczba(std::vector<uint32_t> liczba){
        slowa = liczba;
    }
};
// 1 gdy a wieksze, -1 gdy b wieksze 0 gdy rowne
int porownaj(WielkaLiczba& a, WielkaLiczba& b){
    if (a.slowa.size() > b.slowa.size()) return 1;
    if (a.slowa.size() < b.slowa.size()) return -1;

    for (int i = 0; i < (int)a.slowa.size(); i++){
        if (a.slowa[i] > b.slowa[i]) return 1;
        if (a.slowa[i] < b.slowa[i]) return -1;
    }

    return 0;
}
//znak roznicy a - b (wielkich liczb)
int znak_roznicy(WielkaLiczba& a, WielkaLiczba& b){
    if (a.slowa.size() > b.slowa.size()) return 1;
    if (a.slowa.size() < b.slowa.size()) return -1;

    for (int i=0;i<(int)a.slowa.size();i++){
        if (a.slowa[i] > b.slowa[i]) return 1;
        if (a.slowa[i] < b.slowa[i]) return -1;
    }
    return 1; //dla zera +
}

WielkaLiczba mnoz_skalar(WielkaLiczba& a, uint32_t b){
    std::vector<uint32_t> wynik;
    int i = (int)a.slowa.size()-1;
    uint64_t iloczyn;
    uint64_t carry = 0;
    

    while (i >= 0){
        uint32_t ai = a.slowa[i];
        iloczyn = (uint64_t)ai * b + carry;
        wynik.push_back((uint32_t)(iloczyn & 0xFFFFFFFF));
        carry = iloczyn >> 32;

        i--;
    }
    if (carry){
        wynik.push_back((uint32_t)(carry));
    }
    std::reverse(wynik.begin(), wynik.end());
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
        WielkaLiczba a = WielkaLiczba(v1);
        WielkaLiczba b = WielkaLiczba(v2);

        return {a, b};
    } else {
        throw std::runtime_error("Wszystkie liczby z pliku zostały użyte");
    }
}

WielkaLiczba operator<<(const WielkaLiczba& a, int k)
{
    if (k == 0) return a;

    if (k % 32 != 0) {
        std::cout << "Ten operator obsluguje tylko wielokrotnosci 32\n";
        exit(1);
    }

    int word_shift = k / 32;

    std::vector<uint32_t> wynik = a.slowa;

    // MSB-first → dokładamy zera NA KOŃCU
    wynik.insert(wynik.end(), word_shift, 0);

    return WielkaLiczba(wynik);
}
// dodawanie piosemne
WielkaLiczba dodawanie(WielkaLiczba& a, WielkaLiczba& b){
    std::vector<uint32_t> wynik;
    wynik.reserve(std::max(a.slowa.size(), b.slowa.size())+1);
    uint64_t carry = 0;
    int i = (int)a.slowa.size() - 1;
    int j = (int)b.slowa.size() - 1;
    
    while (i >= 0 || j>=0){
        uint64_t ai = (i >= 0) ? a.slowa[i] : 0;
        uint64_t bj = (j >= 0) ? b.slowa[j] : 0;

        uint64_t sum = ai + bj + carry;
        wynik.push_back((uint32_t)(sum & 0xFFFFFFFF));
        carry = sum >> 32;

        i--;
        j--;
    }
    if (carry){
        wynik.push_back(carry);
    }
    std::reverse(wynik.begin(), wynik.end());
    return WielkaLiczba(wynik);
}
// wyswietlanie
void print_words(const WielkaLiczba& a)
{
    std::cout << "[ ";

    for (auto x : a.slowa)
    {
        std::cout << x << " ";
    }

    std::cout << "]\n";
}

// odejmowanie pisemne a - b: wieksza -  mniejsza!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
WielkaLiczba odejmij(WielkaLiczba& a, WielkaLiczba& b){
    std::vector<uint32_t> wynik;
    int i = (int)a.slowa.size()-1;
    int j = (int)b.slowa.size()-1;
    uint64_t borrow = 0;

    while(i >= 0 || j >= 0){
        uint64_t ai = (i>=0) ? a.slowa[i] : 0;
        uint64_t bi = (j>=0) ? b.slowa[j] : 0;

        uint64_t do_odjecia = bi + borrow;
        if (ai < do_odjecia){
            wynik.push_back((uint32_t)(ai + (1ULL << 32) - do_odjecia));
            borrow = 1;
        } else {
            wynik.push_back((uint32_t)(ai - bi - borrow));
            borrow = 0;
        }
        i--;
        j--;
    }
    //usuniecie wiodacych zer
    while (wynik.size() > 1 && wynik.back() == 0){
        wynik.pop_back();
    }
    std::reverse(wynik.begin(), wynik.end());
    return WielkaLiczba(wynik);
}
//moduł roznicy A-B
WielkaLiczba modul_roz(WielkaLiczba& a, WielkaLiczba& b){
    int znak = znak_roznicy(a, b);
    if (znak==1){
        return odejmij(a, b);
    } 
    return odejmij(b,a);
}
//modulo beta^k
WielkaLiczba mod_podst_k(WielkaLiczba& a, int k){
    int n = (int)a.slowa.size();
    if (k >= n){
        return a;
    }
    std::vector<uint32_t> wynik(
        a.slowa.end()-k,
        a.slowa.end()
    );
    return WielkaLiczba(wynik);
}

// dzielenie całkowite wielkiej liczby przez potęgę 2^32
WielkaLiczba dziel_calk_pot(WielkaLiczba& a, int k){
    int n = (int)a.slowa.size();
    if (k >= n){
        return WielkaLiczba({0});
    }
    std::vector<uint32_t> wynik(
        a.slowa.begin(),
        a.slowa.end()-k
    );
    return WielkaLiczba(wynik);
}
// mnozenie poisemne 
WielkaLiczba mnoz_pisemne(WielkaLiczba& a,WielkaLiczba& b){
    int i = 1;
    int m = (int)b.slowa.size()-1;
    WielkaLiczba wynik = mnoz_skalar(a, b.slowa[m]);
    WielkaLiczba temp;
    m--;
    while (m >= 0){
        temp = mnoz_skalar(a, b.slowa[m]);
        temp = temp << (i * 32);
        wynik = dodawanie(wynik, temp);
        i++;
        m--;
    }
    return wynik;
}

WielkaLiczba div_exact(const WielkaLiczba& a, uint32_t d)
{
    std::vector<uint32_t> wynik;
    wynik.reserve(a.slowa.size());

    uint64_t rem = 0;

    for (size_t i = 0; i < a.slowa.size(); i++)
    {
        uint64_t cur = (rem << 32) | a.slowa[i];
        //std::cout<<"i="<<i<<", cur="<<cur<<"\n";
        uint32_t q = (uint32_t)(cur / d);
        //std::cout<<"i="<<i<<", q="<<q<<"\n";
        rem = cur % d;

        wynik.push_back(q);
    }

    // tutaj zakładamy exact division
    if (rem != 0)
    {
        std::cout << "blad: dzielenie nie jest exact\n";
        std::cout << "a:";print_words(a);
        std::cout<< "\n dzielnik" << d <<"\n";
    }

    // usuwanie zer wiodących
    int i = 0;
    // -1 żeby ostatnie 0 zostało!! (niżej)
    while (i < (int)wynik.size() - 1 && wynik[i] == 0)
        i++;

    if (i > 0){
        wynik.erase(wynik.begin(), wynik.begin()+i);
    }
    //proba
    
    return WielkaLiczba(wynik);
}
// mnozenie karatsuby (te same dlugosci liczb)!!!!!!!!!!!!!!!
WielkaLiczba mnoz_karatsuba(WielkaLiczba& a,WielkaLiczba& b){
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
    WielkaLiczba X = modul_roz(A0,A1);
    WielkaLiczba Y = modul_roz(B0, B1);
    WielkaLiczba C2 = mnoz_karatsuba(X, Y);// czy X i Y maja taki sam rozmiar???????? -> nie zawsze maja - czy cos z tym zrobić? -> bo sie zera usuwaja
    WielkaLiczba wynik = C0;
    if (s_a * s_b == -1){
        C2 = dodawanie(C2, C1);
        C2 = dodawanie(C2, C0);
        C2 = C2 << (k * 32);
        C1 = C1 << (2*k*32);
    } else {
        WielkaLiczba wsp = dodawanie(C1, C0);
        int q = porownaj(wsp, C2);
        if (q == -1){
            std::cout<<"blad w mnozeniu karatsuby"<<std::endl;
        }
        C2 = odejmij(wsp, C2);
        C2 = C2 << (k * 32);
        C1 = C1 << (2 * k * 32);

    }
    wynik = dodawanie(wynik, C2);
    wynik = dodawanie(wynik, C1);
    return wynik;
}

//Toom-Cook-3 - ten sam rozmiar
WielkaLiczba mnoz_toomcook3(WielkaLiczba& a, WielkaLiczba& b){
    if (((a.slowa.size()==1) && (a.slowa[0]==(uint32_t)0)) || ((b.slowa.size()==1)&&(b.slowa[0]==(uint32_t)0))){
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
    if (znak==1){
        temp1 = odejmij(a02, a1);
    } else {
        temp1 = odejmij(a1, a02);
        temp1.czy_ujemna = true;
    }
    znak = znak_roznicy(b02, b1);
    if (znak==1){
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
    temp2.czy_ujemna = false; //zresetowanie znaku bo juz nie potrzebny
    //v-1 zrobione
    temp1 = mnoz_skalar(a2, (uint32_t)4);
    temp2 = mnoz_skalar(b2, (uint32_t)4);
    WielkaLiczba temp3 = mnoz_skalar(a1, (uint32_t)2);
    WielkaLiczba temp4 = mnoz_skalar(b1, (uint32_t)2);

    temp3 = dodawanie(temp3, temp1);
    temp4 = dodawanie(temp4, temp2);
    temp3 = dodawanie(temp3, a0);
    temp4 = dodawanie(temp4, b0);
    WielkaLiczba v2 = mnoz_toomcook3(temp3, temp4);

    WielkaLiczba v_n = mnoz_toomcook3(a2, b2);

    temp1 = mnoz_skalar(v0, (uint32_t)3);
    temp2 = mnoz_skalar(v_1, (uint32_t)2);
    //WielkaLiczba t1 = dodawanie(temp1, temp2);
    WielkaLiczba t1;
    if (v_1.czy_ujemna){
        znak = znak_roznicy(temp1, temp2);
        if (znak==1){
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
        if (znak==1){
            t1 = odejmij(v2, t1);
            t1.czy_ujemna = false;
        } else {
            t1 = odejmij(t1, v2);
            t1.czy_ujemna = true;
        }
    } else {
        t1 = dodawanie(t1, v2);
    }
    //t1 = dodawanie(t1, v2);///tutaj jeszcze zobaczycccc!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    t1 = div_exact(t1, (uint32_t)6);
    temp3 = mnoz_skalar(v_n, (uint32_t)2);// dzielenia mozna potem zmienic na przesuniecia bitowe tam gdzie przez dwa sie dzieli!!!
    if (t1.czy_ujemna){
        t1 = dodawanie(t1, temp3);//bo temp3 jest na pewno dodatnie wiec wynik na pewno ujemny
    
    } else {
        znak = znak_roznicy(t1, temp3);
        if (znak==1){
            t1 = odejmij(t1, temp3);
        } else {
            t1 = odejmij(temp3, t1);
            t1.czy_ujemna = true;
        }
    }
    // t1 zrobione
    
    WielkaLiczba t2;
    if (v_1.czy_ujemna){
        znak = znak_roznicy(v1, v_1);
        if (znak==1){
            t2 = odejmij(v1, v_1);
        } else {
            t2 = odejmij(v_1, v1);
            t2.czy_ujemna = true;
        }
    } else {
        t2 = dodawanie(v1, v_1);
    }
    //t2 zrobione
    t2 = div_exact(t2, 2);

    //c0 to v0 !!!
    WielkaLiczba c1;// = odejmij(v1, t1); tu juz musi wyjsc dodatni wiec bez znaku roznicy sprawdzania
    if (t1.czy_ujemna){
        c1 = dodawanie(v1, t1);
    } else {
        c1 = odejmij(v1, t1);
    }//c1 zrobione.
    WielkaLiczba c2;// = odejmij(t2, v0); v0 i v_n zawsze dodatnie wiec t2 musi byc na plusie - bez warunkow
    if (t2.czy_ujemna){
        std::cout<<"cos nie tak w obliczeniach c2 w toomiecooku3\n";
    }
    c2 = odejmij(t2, v0);
    c2 = odejmij(c2, v_n); //c2 powinno byc git
    WielkaLiczba c3 = odejmij(t1, t2); //tez bez warunkow bo powinno dodatnie wyjsc czyli zarowno t1 i t2 musza byc dodatnie (t2 z poprzedniej czesci).
    //c4 to v_n

    c1 = c1 << (k*32);
    c2 = c2 << (k*2*32);
    c3 = c3 << (k*3*32);
    v_n = v_n << (k*4*32);

    v0 = dodawanie(v0, c1);
    v0 = dodawanie(v0, c2);
    v0 = dodawanie(v0, c3);
    v0 = dodawanie(v0, v_n);

    return v0;//zwraca wielką liczbę!
}
/*
//funkcja do testowania - na razie ją zostawiam
int main(){
    std::ifstream file("Dane_wygenerowane/data_1000.txt");

    auto [a, b] = wczytaj_dwie_liczby(file);
    //WielkaLiczba c({1,2,6});
    //uint32_t t = (uint32_t) 6;
    //print_words(a);
    std::cout<<"\n----------------------------------------------------\n";
    //print_words(b);
    std::cout<<"\n----------------------------------------------------\n";
    WielkaLiczba wynik = mnoz_toomcook3(a,b);
    print_words(wynik);
    
    WielkaLiczba wynik = mnoz_karatsuba(a,b); - karatsuba git liczy
    print_words(wynik);
    
    //WielkaLiczba wynik = div_exact(c, t);
    //print_words(wynik);

    file.close();
    return 0;
} */

struct TimeResult {
    double value;
    std::string unit;
};

TimeResult choose_time_unit(double input_ns) {
    TimeResult result;
    if (input_ns < 1000) {
        result.value = input_ns; // nanoseconds
        result.unit = "ns";
    } else if (input_ns < 1'000'000) {
        result.value = input_ns / 1000; // microseconds
        result.unit = "us";
    } else if (input_ns < 1'000'000'000) {
        result.value = input_ns / 1'000'000; // milliseconds
        result.unit = "ms";
    } else {
        result.value = input_ns / 1'000'000'000; // seconds
        result.unit = "s";
    }
    return result;
}

int main(){
    std::vector<size_t> sizes = {50, 100, 150, 200, 250, 300, 350, 400, 450, 500,
        550, 600, 650, 700, 750, 800, 850, 900, 950, 1000}; //vector of sizes as numbers of words
    std::string output_csv = "resultsToom.csv";
    std::ofstream file_csv(output_csv, std::ios::app);
    if (!file_csv){
        std::cout<<"error opening resultsToom.csv file.\n";
        return 1;
    }
    
    for (size_t size : sizes){
        std::string input_filename = "Dane_wygenerowane/data_" + std::to_string(size) + ".txt";
        std::ifstream file(input_filename);
        if (!file){
            std::cout<<"Error opening file with numbers\n";
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

            size_t num_words = vec1.size(); //dwa maja taki sam rozmiar!!

            WielkaLiczba a(vec1);
            WielkaLiczba b(vec2);

            auto start = std::chrono::high_resolution_clock::now();
            WielkaLiczba wynik = mnoz_toomcook3(a,b);
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop-start).count();

            TimeResult time_result = choose_time_unit(duration);
            std::cout << "mpz_mul execution time: " << time_result.value << " " << time_result.unit << std::endl;
            file_csv << num_words << "," << time_result.value << "," << time_result.unit << std::endl;

        }
        file.close();
    }
    file_csv.close();
}



