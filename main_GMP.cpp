#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <gmp.h>
#include <cstdint>
#include <chrono>



void vector_to_mpz(mpz_t result, const std::vector<uint32_t>& vec){
    // Convert the vector of uint32_t to an mpz_t number
    // '1' means that the most significant word is at the start of the vector
        mpz_import(result, vec.size(), 1, sizeof(uint32_t), 0, 0, vec.data());
    }




////////////////////////////////////////
//
// MAIN
//
/////////////////////////////////////////

int main() {
    std::ifstream file("data_500.txt");
    if (!file) {
        std::cout << "Error opening file!" << std::endl;
        return 1;
    }

    std::ofstream csv_file("resultsGMP.csv", std::ios::app);
    if (!csv_file) {
        std::cout << "Error opening CSV file!" << std::endl;
        return 1;
    }

    std::string line1, line2;

    while(std::getline(file, line1) && std::getline(file, line2)){
        std::vector<uint32_t> vec1, vec2;
        uint32_t word;

        std::stringstream ss1(line1);
        while (ss1 >> word) {
            vec1.push_back(word);
        }

        std::stringstream ss2(line2);
        while (ss2 >> word) {
            vec2.push_back(word);
        }

        mpz_t num1, num2, result;
        mpz_init(num1);
        mpz_init(num2);
        mpz_init(result);

        vector_to_mpz(num1, vec1);
        vector_to_mpz(num2, vec2);

        // Measure time of mpz_mul execution
        auto start = std::chrono::high_resolution_clock::now();
        mpz_mul(result, num1, num2);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        
        std::cout << "mpz_mul execution time: " << duration.count() << " microseconds" << std::endl;
        csv_file << "500," << duration.count() << std::endl;

        // Print the results
        /*
        std::cout << "First number: " << mpz_get_str(NULL, 10, num1) << std::endl;
        std::cout << "Second number: " << mpz_get_str(NULL, 10, num2) << std::endl;
        std::cout << "Result: " << mpz_get_str(NULL, 10, result) << std::endl;
        std::cout << "-----------------------------" << std::endl;
        */

        mpz_clear(num1);
        mpz_clear(num2);
        mpz_clear(result);
    }

    file.close();
    csv_file.close();

    return 0;
}
