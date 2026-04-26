#include<iostream>
#include<vector>
#include<cstdint>
#include<chrono>
#include<fstream>
#include<sstream>
#include<string>
#include<chrono>

struct WielkaLiczbaSlow {
    std::vector<uint32_t> slowa;
    bool czy_ujemna = false;

    //konstruktor domyslny
    WielkaLiczbaSlow() {};

    WielkaLiczbaSlow(std::vector<uint32_t> liczba) {
        slowa = liczba; 
    }
};

struct WielkaLiczbaFast {
    std::vector<uint32_t> slowa;
    bool czy_ujemna = false;

    //konstruktor domyslny
    WielkaLiczbaFast() {};

    WielkaLiczbaFast(const std::vector<uint32_t> &liczba) {
        slowa = liczba;
    }
};



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

    //load vector one vector from file
    std::vector<uint32_t> v;
    std::ifstream file("Dane_wygenerowane/data_50.txt");
    if (!file){
        std::cout << "error opening test.txt file.\n";
        return 1;
    }
    std::string line;
    if (std::getline(file, line)){
        std::stringstream ss(line);
        uint32_t word;
        while (ss >> word){
            v.push_back(word);
        }
    } else {
        std::cout << "error reading line from test.txt file.\n";
        return 1;
    }

    const int num_iterations = 100000;

    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < num_iterations; i++){
        WielkaLiczbaSlow slow(v);
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    
    TimeResult time_result = choose_time_unit(duration);

    std::cout << "Time taken to create " << num_iterations << " WielkaLiczbaSlow objects: " << time_result.value << " " << time_result.unit << "\n";

     start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < num_iterations; i++){
        WielkaLiczbaFast fast(v);
    }
     stop = std::chrono::high_resolution_clock::now();
     duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();

    TimeResult time_result2 = choose_time_unit(duration);
    std::cout << "Time taken to create " << num_iterations << " WielkaLiczbaFast objects: " << time_result2.value << " " << time_result2.unit << "\n";

    std::cout << "\n\n\n";

    const int N = 100; // number of zeros to insert
    std::vector<uint32_t> temp = v;

    start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < N; i++){
        temp.insert(temp.begin(), 0);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    TimeResult time_result3 = choose_time_unit(duration);
    std::cout << "Time taken to insert " << N << " zeros at the beginning of a vector: " << time_result3.value 
        << " " << time_result3.unit << " Big-endian\n"; 

    temp = v;
    start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < N; i++){
        temp.push_back(0);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    TimeResult time_result4 = choose_time_unit(duration);
    std::cout << "Time taken to insert " << N << " zeros at the end of a vector: " << time_result4.value 
        << " " << time_result4.unit << " Little-endian\n";

    return 0;
}