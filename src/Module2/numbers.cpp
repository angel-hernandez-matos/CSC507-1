/* Compile with: g++ -std=c++17 numbers.cpp -o numbers */

#include <iostream>
#include <fstream>
#include <filesystem>
#include <random>
#include <algorithm>
#include <iterator>
#include <chrono>   

namespace fs = std::filesystem;

int main() {
    const int items_to_create = 1000;
    const char* filename = "file2.txt";  
    const char* resultDirectory = "results";      

    // Remove file2.txt if it exists from a previous run 
    if (fs::exists(filename)) {
        fs::remove(filename);
    }

    // Start timing
    auto start = std::chrono::high_resolution_clock::now();

    std::minstd_rand rng(11111); 
    std::ofstream outfile(filename);      
    std::uniform_int_distribution<int> dist(0, 10000);

    auto generateRandom = [&]() { return dist(rng); }; 

    // Generate and write 1000 random numbers
    std::generate_n(std::ostream_iterator<int>(outfile, "\n"), 
                    items_to_create, 
                    generateRandom);

    outfile.close();

    // End timing
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "File '" << filename << "' created with 1000 random numbers.\n";
    std::cout << "Random number generator is complete. Time taken: " << elapsed << " ms\n";
    
    fs::create_directories(resultDirectory);

    // Count lines and words in file2.txt
    auto lineCount = 0;
    auto wordCount = 0;
    std::string line;
    std::ifstream infile(filename);

    while (std::getline(infile, line)) {
        ++lineCount;
        std::istringstream iss(line);
        std::string word;
        while (iss >> word) {
            ++wordCount;
        }
    }
    
    infile.close();

    // Get current date/time
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    // Write results to wc_cpp
    std::ofstream wcfile("results/wc_cpp.txt", std::ios::app); // append mode
    wcfile << "Date: " << std::ctime(&now_c);              // ctime adds newline automatically
    wcfile << "Line count: " << lineCount << "\n";
    wcfile << "Word count: " << wordCount << "\n\n";
    wcfile.close();

    std::cout << "Word/line count written to results/wc_cpp.txt\n";

    return 0;
}

