# File: mainCT4.py
# Written by: Angel Hernandez
# Description: Module 4 - Critical Thinking
# Requirement(s): First-Fit Algorithm Simulation

import os

class FirstFitSimulator:
    def perform_allocation(self, blocksize, processes):
        if blocksize is None or processes is None:
            raise Exception('Block size and number of processes do not match')

        # Keep a copy of the original block sizes
        originalsize = blocksize.copy()
        allocation = [-1] * len(processes)

        # First Fit allocation
        for x in range(len(processes)):
            for y in range(len(blocksize)):
                if blocksize[y] >= processes[x][1]:
                    allocation[x] = y
                    blocksize[y] -= processes[x][1]
                    break

        print('Process Name\tSize\tBlock Number\tOriginal Size\tAfter Allocation')
        for i in range(len(processes)):
            if allocation[i] != -1:
                block_num = allocation[i] + 1
                originalalloc = originalsize[allocation[i]]
                afteralloc = blocksize[allocation[i]]
            else:
                block_num = "Unallocated"
                originalalloc = "-"
                afteralloc = "-"
            print(f'{processes[i][0]}\t{processes[i][1]}\t{block_num}\t\t{originalalloc}\t\t{afteralloc}')

class TestCaseRunner:
    @staticmethod
    def run_test():
        blocksize = [100, 500, 200, 300, 600]
        processes = [('Process 1', 200), ('Process 2', 400), ('Process 3', 100), ('Process 4', 450)]

        simulator = FirstFitSimulator()
        simulator.perform_allocation(blocksize, processes)

def clear_screen():
    command = 'cls' if os.name == 'nt' else 'clear'
    os.system(command)

def main():
    try:
        clear_screen()
        print('*** Module 4 - Critical Thinking ***\n')
        TestCaseRunner.run_test()
    except Exception as e:
        print(e)

if __name__ == '__main__':
    main()
