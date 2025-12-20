/*
  File: rt_diff_reads.cpp
  Written by: Angel Hernandez 
  Description: Module 6 - Critical Thunking
  Requirement: Evaluate real-time style scheduling strategies by splitting
               a large input file into multiple parts and processing them
               in parallel, then combining results.
  Scenarios:
    1) Single run on full file (1 chunk)
    2) 10 chunks processed in parallel
    3)  2 chunks processed in parallel
    4)  5 chunks processed in parallel
    5) 20 chunks processed in parallel

  Compile with:
    g++ -std=c++17 -O2 rt_diff_reads.cpp -o rt_diff_reads
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

using Clock = std::chrono::high_resolution_clock;

struct ScenarioResult {
    std::string name;
    long long elapsed_ms;
    int chunks;
};

void process_file(const std::string& input_file,  const std::string& output_file) {
    std::string line;
    std::ifstream in(input_file);
    std::ofstream out(output_file);
    
    if (!in) {
        std::cerr << "Error: could not open input file " << input_file << "\n";
        return;
    }
    
    if (!out) {
        std::cerr << "Error: could not open output file " << output_file << "\n";
        return;
    }
    
    while (std::getline(in, line)) {
        if (line.empty())
            continue;
            
        long long value = 0;
        std::istringstream iss(line);
        iss >> value;
        out << value << "\n";
    }
}

/* Split the main input file into N chunk files line-by-line. Each chunk gets approximately 
   total_lines / num_chunks lines */
void split_file_linewise(const std::string& input_file,  const std::string& chunk_prefix, int num_chunks) {
    std::ifstream in(input_file);
    
    if (!in) {
        std::cerr << "Error: could not open input file for splitting: "
                  << input_file << "\n";
        return;
    }

    std::vector<std::ofstream> chunk_streams;
    chunk_streams.reserve(num_chunks);

    for (auto i = 0; i < num_chunks; ++i) {
        std::ostringstream oss;
        oss << chunk_prefix << "_part_" << i << ".txt";
        chunk_streams.emplace_back(oss.str());
        
        if (!chunk_streams.back()) {
            std::cerr << "Error: could not create chunk file: " << oss.str() << "\n";
            return;
        }
    }

    /* Simple round-robin distribution of lines across chunks */
    std::string line;
    auto current_chunk = 0;
    
    while (std::getline(in, line)) {
        chunk_streams[current_chunk] << line << "\n";
        current_chunk = (current_chunk + 1) % num_chunks;
    }
}

/* Combine multiple partial output files into a single file in order */
void combine_outputs(const std::string& combined_output_file, const std::string& chunk_output_prefix, int num_chunks) {
    std::ofstream out(combined_output_file);
    if (!out) {
        std::cerr << "Error: could not open combined output file: "
                  << combined_output_file << "\n";
        return;
    }

    for (auto i = 0; i < num_chunks; ++i) {
        std::ostringstream oss;
        oss << chunk_output_prefix << "_part_" << i << ".txt";

        std::ifstream in(oss.str());
        if (!in) {
            std::cerr << "Warning: could not open partial output file: "
                      << oss.str() << "\n";
            continue;
        }

        std::string line;
        while (std::getline(in, line)) {
            out << line << "\n";
        }
    }
}

/* Run a scenario with given number of chunks, using parallel processing. For num_chunks == 1, this is 
   equivalent to a single run on the full file */
ScenarioResult run_scenario(const std::string& input_file, const std::string& scenario_name,
                            const std::string& scenario_output_prefix, int num_chunks) {
    ScenarioResult result;
    result.name = scenario_name;
    result.chunks = num_chunks;

    // Directory for intermediate files
    const std::string temp_dir = "results/tmp_" + scenario_name;
    fs::create_directories(temp_dir);

    // For num_chunks == 1 we can skip explicit splitting and just process directly.
    auto start = Clock::now();

    if (num_chunks == 1) {
        std::string output_file = "results/" + scenario_output_prefix + "_output.txt";
        process_file(input_file, output_file);
    } else {
        // 1) Split into chunk input files
        std::string chunk_input_prefix = temp_dir + "/input";
        split_file_linewise(input_file, chunk_input_prefix, num_chunks);

        // 2) Launch threads to process each chunk input -> chunk output
        std::vector<std::thread> threads;
        std::mutex cout_mutex;

        for (auto i = 0; i < num_chunks; ++i) {
            threads.emplace_back([&, i]() {
                std::ostringstream in_name, out_name;
                in_name  << chunk_input_prefix  << "_part_" << i << ".txt";
                out_name << temp_dir << "/output_part_" << i << ".txt";

                process_file(in_name.str(), out_name.str());

                {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::cout << "Scenario " << scenario_name
                              << " - thread " << std::this_thread::get_id()
                              << " processed chunk " << i << ".\n";
                }
            });
        }

        for (auto& th : threads) {
            th.join();
        }

        // 3) Combine partial outputs in order
        std::string combined_output_file = "results/" + scenario_output_prefix + "_output.txt";
        std::string chunk_output_prefix = temp_dir + "/output";
        combine_outputs(combined_output_file, chunk_output_prefix, num_chunks);
    }

    auto end = Clock::now();
    result.elapsed_ms =  std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    return result;
}

/* Program's entry point */
int main() {
    const std::string input_file = "file1.txt";   // Expect ~10,000,000 lines
    const std::string results_dir = "results";
    fs::create_directories(results_dir);

    CLEAR_SCREEN();
    std::cout << "Module 6 (Real-Time Scheduling) - "
                 "Benchmarking chunked processing strategies...\n\n";

    std::vector<ScenarioResult> results;

    // (a) Run as is (single large file)
    std::cout << "Running scenario (1): 1 chunk, full file...\n";
    results.push_back(run_scenario(input_file, "1", "scenario_1", 1));

    // (b) Break into 10 files
    std::cout << "\nRunning scenario (2): 10 chunks...\n";
    results.push_back(run_scenario(input_file, "2", "scenario_2", 10));

    // (c) Break into 2 files
    std::cout << "\nRunning scenario (3): 2 chunks...\n";
    results.push_back(run_scenario(input_file, "3", "scenario_3", 2));

    // (d) Break into 5 files
    std::cout << "\nRunning scenario (4): 5 chunks...\n";
    results.push_back(run_scenario(input_file, "4", "scenario_4", 5));

    // (e) Break into 20 files
    std::cout << "\nRunning scenario (5): 20 chunks...\n";
    results.push_back(run_scenario(input_file, "5", "scenario_5", 20));

    std::cout << "\n=== Summary of elapsed times ===\n";
    for (const auto& r : results) {
        std::cout << "Scenario " << r.name
                  << " (" << r.chunks << " chunk(s)): "
                  << r.elapsed_ms << " ms\n";
    }

    // Write summary to file
    std::ofstream summary(results_dir + "/rt_scheduling_benchmark.txt", std::ios::app);
    if (summary) {
        summary << "Real-Time Scheduling Benchmark Results:\n";
        for (const auto& r : results) {
            summary << "Scenario " << r.name
                    << " (" << r.chunks << " chunk(s)): "
                    << r.elapsed_ms << " ms\n";
        }
        summary << "\n";
    }

    std::cout << "\nBenchmark complete. Detailed outputs are in the 'results' directory.\n";
    
    return 0;
}
