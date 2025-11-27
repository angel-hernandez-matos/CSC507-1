/*
  File: numbers.cpp
  Written by: Angel Hernandez
  Description: Module 3 - Portfolio Milestone
  Requirement(s): Random generation of 1,000,000 numbers using multiple threads
  Compile with: g++ -std=c++17 numbers.cpp -o numbers 
*/

#include <iostream>
#include <fstream>
#include <filesystem>
#include <random>
#include <vector>
#include <chrono>
#include <sstream>
#include <thread>
#include <map>
#include <mutex>

namespace fs = std::filesystem;

#define CLEAR_SCREEN() std::cout << "\033[2J\033[H"

int main() {
    const int items_to_create = 1000000;   // 1 million numbers
    const char* filename = "file2.txt";  
    const char* resultDirectory = "results";   
    CLEAR_SCREEN();
    std::cout << "Portfolio Milestone 3 - Start generation of  random numbers - C++ Implementation...\n";

    if (fs::exists(filename)) 
        fs::remove(filename);

    auto start = std::chrono::high_resolution_clock::now();

    // Vector to hold random numbers
    std::vector<int> numbers(items_to_create);

    // Track threads used
    std::mutex map_mutex;
    std::map<std::thread::id, bool> threads;
    
    // Decide how many threads to use
    auto num_threads = std::thread::hardware_concurrency();
    
    if (num_threads == 0) // fallback if hardware_concurrency not available
        num_threads = 4; 

    // Worker function
    auto worker = [&](int start, int end) {
        std::uniform_int_distribution<int> dist(0, 10000);
        static thread_local std::minstd_rand rng(std::random_device{}());        

        for (auto i = start; i < end; ++i) 
            numbers[i] = dist(rng);

        auto tid = std::this_thread::get_id();
        {
            std::lock_guard<std::mutex> lock(map_mutex);
            if (threads.find(tid) == threads.end()) {
                threads[tid] = true;
                std::cout << "Thread " << tid << " is generating random numbers.\n";
            }
        }
    };

    // Spawn threads    
    auto start_idx = 0;
    std::vector<std::thread> thread_pool;
    auto chunk_size = items_to_create / num_threads;
    
    for (unsigned int t = 0; t < num_threads; ++t) {
        auto end_idx = (t == num_threads - 1) ? items_to_create : start_idx + chunk_size;
        thread_pool.emplace_back(worker, start_idx, end_idx);
        start_idx = end_idx;
    }

    // Join threads
    for (auto& t : thread_pool) 
        t.join();

    std::cout << threads.size() << " thread(s) were created.\n";

    // Write results sequentially
    {
        std::ofstream outfile(filename);
        for (auto num : numbers) {
            outfile << num << "\n";
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Break down into hours, minutes, seconds, milliseconds
    auto elapsed_seconds = elapsed_ms / 1000;
    auto hours = elapsed_seconds / 3600;
    auto minutes = (elapsed_seconds % 3600) / 60;
    auto seconds = elapsed_seconds % 60;
    auto milliseconds = elapsed_ms % 1000;

    // Store formatted time in a string
    auto time_taken = "Time taken: " + 
                         std::to_string(hours) + "h " + 
                         std::to_string(minutes) + "m " + 
                         std::to_string(seconds) + "s " +
                         std::to_string(milliseconds) + "ms\n";

    std::cout << "File '" << filename << "' created with " << items_to_create << " random numbers.\n";
    std::cout << "Random number generator is complete. " << time_taken;

    fs::create_directories(resultDirectory);

    // Count lines and words
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

    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);

    std::ofstream wcfile("results/wc_cpp.txt", std::ios::app);
    wcfile << "Date: " << std::ctime(&now_c);
    wcfile << "Line count: " << lineCount << "\n";
    wcfile << "Word count: " << wordCount << "\n";
    wcfile << time_taken << "\n";
    wcfile.close();

    std::cout << "Word/line count written to results/wc_cpp.txt\n";

    return 0;
}
