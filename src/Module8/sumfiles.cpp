/*
  File: sumfiles.cpp
  Written by: Angel Hernandez
  Description: Module 8 - Portfolio Project
  Requirement: Create a program, using a programming language of your choice, to produce a new file: totalfile.txt,
  by taking the numbers from each line of the two files and adding them. So, each line in file #3 is the sum of the
  corresponding line in hugefile1.txt and hugefile2.txt.

  This program has different execution modes based on the arguments.

  Usage examples:
    - Full (single-thread, whole files): ./sumfiles file1 file2 outfile full\
    - Range (single-thread, for halves): ./sumfiles file1 file2 outfile range start count
    - Chunk (single-thread, for pre-split chunks): ./sumfiles chunk1 chunk2 outfile chunk
    - Threaded (multi-thread, N chunks/threads): ./sumfiles file1 file2 outfile threaded num_threads

  Compile with:
    g++ -O3 -std=c++17 sumfiles.cpp -o sumfiles
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <filesystem>
#include <chrono>
#include <cstdlib>

namespace fs = std::filesystem;
using Clock = std::chrono::high_resolution_clock;

// --------------------- BASIC SINGLE-THREAD MODES ------------------------

void sum_full(const std::string &f1, const std::string &f2, const std::string &out) {
    std::string line1, line2;
    std::ofstream outFile(out);
    std::ifstream in1(f1), in2(f2);

    if (!in1 || !in2 || !outFile) {
        std::cerr << "Error: cannot open files in full mode\n";
        std::cerr << "The selected files are [F1]]: " << f1 <<  " [F2]: " << f2 << " [OUT]: " << out << "\n";
        return;
    }

    while (std::getline(in1, line1) && std::getline(in2, line2)) {
        auto val1 = std::stoll(line1);
        auto val2 = std::stoll(line2);
        outFile << (val1 + val2) << "\n";
    }
}

void sum_range(const std::string &f1, const std::string &f2, const std::string &out,
               long long start, long long count) {
    std::string line1, line2;
    std::ofstream outFile(out);
    std::ifstream in1(f1), in2(f2);

    if (!in1 || !in2 || !outFile) {
        std::cerr << "Error: cannot open files in range mode\n";
        return;
    }

    // Skip to start line (1-based)
    for (auto i = 1; i < start; ++i) {
        if (!std::getline(in1, line1) || !std::getline(in2, line2))
            return;
    }

    // Process 'count' lines
    for (auto i = 0; i < count; ++i) {
        if (!std::getline(in1, line1) || !std::getline(in2, line2))
            break;
        auto val1 = std::stoll(line1);
        auto val2 = std::stoll(line2);
        outFile << (val1 + val2) << "\n";
    }
}

// For pre-split chunks: just sum all lines (same as full)
void sum_chunk(const std::string &f1, const std::string &f2, const std::string &out) {
    sum_full(f1, f2, out);
}

// --------------------- UTILITIES FOR THREADED MODE ---------------------

long long count_lines(const std::string &file) {
    auto count = 0;
    std::string line;
    std::ifstream in(file);

    if (!in) {
        std::cerr << "Error: cannot open " << file << " for counting\n";
        return 0;
    }

    while (std::getline(in, line))
        ++count;

    return count;
}

// Split both files into num_chunks contiguous, aligned chunks
void split_two_files_contiguous(const std::string &file1, const std::string &file2, const std::string &prefix1,
                                const std::string &prefix2, int num_chunks) {
    auto total = count_lines(file1);

    if (total == 0) {
        std::cerr << "Error: zero lines in " << file1 << "\n";
        return;
    }

    auto total2 = count_lines(file2);

    if (total2 != total) {
        std::cerr << "Warning: file line counts differ ("
                << total << " vs " << total2 << ")\n";
    }

    auto base = total / num_chunks;
    auto extra = total % num_chunks;

    std::ifstream in1(file1), in2(file2);

    if (!in1 || !in2) {
        std::cerr << "Error: cannot reopen input files for splitting\n";
        return;
    }

    std::string line1, line2;

    for (auto i = 0; i < num_chunks; ++i) {
        auto lines_this = base + (i < extra ? 1 : 0);
        std::ostringstream f1, f2;
        f1 << prefix1 << "_part_" << i << ".txt";
        f2 << prefix2 << "_part_" << i << ".txt";
        std::ofstream out1(f1.str()), out2(f2.str());

        if (!out1 || !out2) {
            std::cerr << "Error: cannot create chunk files for chunk " << i << "\n";
            return;
        }

        for (auto j = 0; j < lines_this; ++j) {
            if (!std::getline(in1, line1) || !std::getline(in2, line2))
                return;
            out1 << line1 << "\n";
            out2 << line2 << "\n";
        }
    }
}

// Sum corresponding lines from two chunk files (thread worker)
void sum_pair_file(const std::string &in1, const std::string &in2, const std::string &outFile) {
    std::string line1, line2;
    std::ofstream out(outFile);
    std::ifstream f1(in1), f2(in2);

    if (!f1 || !f2 || !out) {
        std::cerr << "Error: cannot open chunk files for threaded mode\n";
        return;
    }

    bool ok1, ok2 = true;

    // while (true) {
    while (!ok1 || !ok2) {
        ok1 = static_cast<bool>(std::getline(f1, line1));
        ok2 = static_cast<bool>(std::getline(f2, line2));
        /* if (!ok1 || !ok2)
             break; */

        auto val1 = std::stoll(line1);
        auto val2 = std::stoll(line2);
        out << (val1 + val2) << "\n";
    }
}

// Combine partial outputs into final file
void combine_outputs(const std::string &finalFile, const std::string &prefix, int num_chunks) {
    std::ofstream out(finalFile);

    if (!out) {
        std::cerr << "Error: cannot open final output " << finalFile << "\n";
        return;
    }

    for (auto i = 0; i < num_chunks; ++i) {
        std::string line;
        std::ostringstream f;
        f << prefix << "_part_" << i << ".txt";
        std::ifstream in(f.str());

        if (!in) {
            std::cerr << "Warning: cannot open " << f.str() << "\n";
            continue;
        }

        while (std::getline(in, line))
            out << line << "\n";
    }
}

// --------------------------- MAIN / MODES ------------------------------

int main(int argc, char *argv[]) {
    if (argc < 5) {
        std::cerr << "Usage:\n"
                << "  Full (single-thread, whole files):\n"
                << "    ./sumfiles file1 file2 outfile full\n\n"
                << "  Range (single-thread, for halves):\n"
                << "    ./sumfiles file1 file2 outfile range start count\n\n"
                << "  Chunk (single-thread, for pre-split chunks):\n"
                << "    ./sumfiles chunk1 chunk2 outfile chunk\n\n"
                << "  Threaded (multi-thread, N chunks/threads):\n"
                << "    ./sumfiles file1 file2 outfile threaded num_threads\n";

        return 1;
    }

    std::string f1 = argv[1];
    std::string f2 = argv[2];
    std::string out = argv[3];
    std::string mode = argv[4];

    try {
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
    }


    if (mode == "full") {
        sum_full(f1, f2, out);
    } else if (mode == "range") {
        if (argc != 7) {
            std::cerr << "Range mode requires start and count\n";
            return 1;
        }
        auto start = std::stoll(argv[5]);
        auto count = std::stoll(argv[6]);
        sum_range(f1, f2, out, start, count);
    } else if (mode == "chunk") {
        sum_chunk(f1, f2, out);
    } else if (mode == "threaded") {
        if (argc != 6) {
            std::cerr << "Threaded mode requires num_threads\n";
            return 1;
        }
        auto num_threads = std::stoi(argv[5]);
        if (num_threads <= 0) {
            std::cerr << "num_threads must be > 0\n";
            return 1;
        }

        // 1) Create temp directory for chunks
        fs::path outPath(out);
        fs::path parent = outPath.parent_path();
        if (parent.empty()) parent = ".";
        fs::path tempDir = parent / ("tmp_threads_" + std::to_string(num_threads));
        fs::create_directories(tempDir);
        std::string prefix1 = (tempDir / "huge1").string();
        std::string prefix2 = (tempDir / "huge2").string();
        std::string outPrefix = (tempDir / "total").string();
        auto startTime = Clock::now();

        // 2) Split both big files into num_threads contiguous chunks
        split_two_files_contiguous(f1, f2, prefix1, prefix2, num_threads);

        // 3) Launch one thread per chunk
        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (auto i = 0; i < num_threads; ++i) {
            std::ostringstream in1, in2, outChunk;
            in1 << prefix1 << "_part_" << i << ".txt";
            in2 << prefix2 << "_part_" << i << ".txt";
            outChunk << outPrefix << "_part_" << i << ".txt";

            threads.emplace_back(sum_pair_file, in1.str(), in2.str(), outChunk.str());
        }

        for (auto &t: threads)
            t.join();

        // 4) Combine chunk outputs into the final outfile
        combine_outputs(out, outPrefix, num_threads);

        auto endTime = Clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        std::cerr << "Threaded mode (" << num_threads << " threads) completed in "
                << ms << " ms\n";
    } else {
        std::cerr << "Unknown mode: " << mode << "\n";
        return 1;
    }

    return 0;
}