/*
  File: diff_reads.cpp
  Written by: Angel Hernandez
  Description: Module 4 - Portfolio Milestone
  Requirement(s): Read file with 1,000,000 nuumbers with 4 different strategies:
                  1-. Read entire file into memory
                  2-. Read one row at a time
                  3-. Split file into 2 parts
                  4-. Multithreaded processing      
  Compile with: g++ -std=c++17 diff_reads.cpp -o diff_reads 
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>
#include <thread>
#include <mutex>

namespace fs = std::filesystem;

#define CLEAR_SCREEN() std::cout << "\033[2J\033[H"

void processRow(const std::string& row) {
    // Example "processing": count words in the row
    std::istringstream iss(row);
    std::string word;
    int count = 0;
    while (iss >> word) {
        ++count;
    }
}

int main() {
    const char* filename = "file1.txt";
    const char* resultDirectory = "results";
    fs::create_directories(resultDirectory);

    CLEAR_SCREEN();
    std::cout << "Portfolio Milestone 4 - Benchmarking file reading strategies...\n";

    /*********************************************
      Method 1: Read entire file into memory
    ******************************************/
    auto start1 = std::chrono::high_resolution_clock::now();

    std::ifstream infile1(filename);
    std::stringstream buffer;
    buffer << infile1.rdbuf();
    std::string fileContents = buffer.str();
    infile1.close();

    std::string row1;
    std::istringstream iss1(fileContents);
    
    while (std::getline(iss1, row1)) {
        processRow(row1);
    }

    auto end1 = std::chrono::high_resolution_clock::now();
    auto elapsed1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count();
    std::cout << "\nMethod 1 (entire file in memory): " << elapsed1 << " ms\n";
    
    /******************************************
      Method 2: Read one row at a time
    ******************************************/
    
    auto start2 = std::chrono::high_resolution_clock::now();

    std::ifstream infile2(filename);
    std::string row2;
    while (std::getline(infile2, row2)) {
        processRow(row2);
    }
    
    infile2.close();

    auto end2 = std::chrono::high_resolution_clock::now();
    auto elapsed2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count();
    std::cout << "Method 2 (row by row): " << elapsed2 << " ms\n";

    /******************************************
      Method 3: Split file into 2 parts
    ******************************************/
    
    auto start3 = std::chrono::high_resolution_clock::now();

    std::ifstream infile3(filename, std::ios::ate | std::ios::binary);
    auto fileSize = infile3.tellg();
    infile3.seekg(0);

    auto halfSize = fileSize / 2;

    std::string part1(halfSize, '\0');
    infile3.read(&part1[0], halfSize);

    std::string part2(fileSize - halfSize, '\0');
    infile3.read(&part2[0], fileSize - halfSize);

    infile3.close();

    std::istringstream iss3a(part1);
    std::string row3a;
    while (std::getline(iss3a, row3a)) {
        processRow(row3a);
    }

    std::istringstream iss3b(part2);
    std::string row3b;
    while (std::getline(iss3b, row3b)) {
        processRow(row3b);
    }

    auto end3 = std::chrono::high_resolution_clock::now();
    auto elapsed3 = std::chrono::duration_cast<std::chrono::milliseconds>(end3 - start3).count();
    std::cout << "Method 3 (split into 2 parts): " << elapsed3 << " ms\n";

    /******************************************
      Method 4: Multithreaded processing
    ******************************************/
    auto start4 = std::chrono::high_resolution_clock::now();

    std::ifstream infile4(filename);
    std::stringstream buffer4;
    buffer4 << infile4.rdbuf();
    std::string fileContents4 = buffer4.str();
    infile4.close();

    // Split into chunks based on number of threads
    auto num_threads = std::thread::hardware_concurrency();
    
    if (num_threads == 0) 
       num_threads = 4;

    std::vector<std::thread> thread_pool;
    std::mutex cout_mutex;

    auto chunk_size = fileContents4.size() / num_threads;

    std::cout << "\nThreads about to start for Method 4 (Multithreaded)...\n";
    auto worker = [&](int start, int end) {
        std::string chunk = fileContents4.substr(start, end - start);
        std::istringstream iss(chunk);
        std::string row;
        while (std::getline(iss, row)) {
            processRow(row);
        }
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Thread " << std::this_thread::get_id() << " processed chunk.\n";
        }
    };

    auto start_idx = 0;
    for (auto t = 0; t < num_threads; ++t) {
        int end_idx = (t == num_threads - 1) ? fileContents4.size() : start_idx + chunk_size;
        thread_pool.emplace_back(worker, start_idx, end_idx);
        start_idx = end_idx;
    }

    for (auto& th : thread_pool) 
      th.join();

    auto end4 = std::chrono::high_resolution_clock::now();
    auto elapsed4 = std::chrono::duration_cast<std::chrono::milliseconds>(end4 - start4).count();
    std::cout << "\nMethod 4 (multithreaded): " << elapsed4 << " ms\n";

    // -------------------------------
    // Write results to file
    // -------------------------------
    std::ofstream outfile("results/file_read_benchmark.txt", std::ios::app);
    outfile << "Benchmark results:\n";
    outfile << "Method 1 (entire file): " << elapsed1 << " ms\n";
    outfile << "Method 2 (row by row): " << elapsed2 << " ms\n";
    outfile << "Method 3 (split file): " << elapsed3 << " ms\n";
    outfile << "Method 4 (multithreaded): " << elapsed4 << " ms\n";
    outfile.close();

    return 0;
}

