/*
  File: vmviewer.cpp
  Written by: Angel Hernandez
  Description: Module 5 - Critical Thinking
  Notes: Utility that displays virtual memory information for a process given its PID or name.
          - Requires PROCESS_QUERY_INFORMATION | PROCESS_VM_READ on target.
          - For best coverage, run as admin when inspecting protected processes.
  Usage:   vmviewer.exe -p 1234
           vmviewer.exe -n notepad.exe
  Options:
    --csv         Output CSV (header included)
    --committed   Show only committed regions

  Compile with: cl /std:c++17 /EHsc /O2 vmviewer.cpp
*/

#include <sstream>
#include <windows.h>
#include <tlhelp32.h>
#include <cstdio>
#include <string>
#include <vector>
#include <optional>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>

#define CLEAR_SCREEN() std::cout << "\033[2J\033[H"

/**
 Struct used to parse arguments from command-line
 **/
struct Arguments {
    bool csv = false;
    std::optional<DWORD> pid;
    bool committedOnly = false;
    std::optional<std::wstring> name;
};

/**
 Shows available command-line arguments
 **/
static void PrintUsage() {
    std::wcout << L"Usage:\n"
               << L"  vmviewer.exe -p <pid> [--csv] [--committed]\n"
               << L"  vmviewer.exe -n <process_name.exe> [--csv] [--committed]\n";
}

/**
 Finds process by name
 **/
static std::optional<DWORD> FindPidByName(const std::wstring& name) {
    auto snapHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapHandle == INVALID_HANDLE_VALUE)
        return std::nullopt;

    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    std::optional<DWORD> retval;

    auto toLower = [](const std::wstring& s) {
        auto out = s;
        std::transform(out.begin(), out.end(), out.begin(), ::towlower);
        return out;
    };

    if (Process32FirstW(snapHandle, &pe)) {
        auto target = toLower(name);

        do {
            auto exe = toLower(pe.szExeFile);
            if (exe == target) {
                retval = pe.th32ProcessID;
                break;
            }
        } while (Process32NextW(snapHandle, &pe));
    }

    CloseHandle(snapHandle);

    return retval;
}


/**
 Memory state mapping WIN32 Constant to description
 **/
static const wchar_t* StateToStr(DWORD state) {
    static const std::unordered_map<DWORD, const wchar_t*> stateMap = {
        { MEM_COMMIT,  L"Commit" },
        { MEM_RESERVE, L"Reserve" },
        { MEM_FREE,    L"Free" }
    };

    auto it = stateMap.find(state);

    return (it != stateMap.end()) ? it->second : L"Unknown";
}

/**
 Memory protection mapping WIN32 Constant to description
 **/
static std::wstring ProtectToStr(DWORD prot, DWORD state) {
    if (state == MEM_FREE)
        return L"N/A";

    static const std::unordered_map<DWORD, const wchar_t*> protMap = {
        { PAGE_NOACCESS,          L"NA" },
        { PAGE_READONLY,          L"R" },
        { PAGE_READWRITE,         L"RW" },
        { PAGE_WRITECOPY,         L"WC" },
        { PAGE_EXECUTE,           L"X" },
        { PAGE_EXECUTE_READ,      L"XR" },
        { PAGE_EXECUTE_READWRITE, L"XRW" },
        { PAGE_EXECUTE_WRITECOPY, L"XWC" }
    };

    auto baseProt = prot & 0xFF;
    auto it = protMap.find(baseProt);
    std::wstring retval = (it != protMap.end()) ? it->second : L"Unknown";

    if (prot & PAGE_GUARD)        retval += L"|G";
    if (prot & PAGE_NOCACHE)      retval += L"|NC";
    if (prot & PAGE_WRITECOMBINE) retval += L"|WCmb";

    return retval;
}

/**
 Memory type mapping WIN32 Constant to description
 **/
static const wchar_t* TypeToStr(DWORD type, DWORD state) {
    if (state == MEM_FREE)
        return L"N/A";

    static const std::unordered_map<DWORD, const wchar_t*> typeMap = {
        { MEM_IMAGE,   L"Image" },
        { MEM_MAPPED,  L"Mapped" },
        { MEM_PRIVATE, L"Private" }
    };
    auto it = typeMap.find(type);

    return (it != typeMap.end()) ? it->second : L"Unknown";
}

/**
 Memory type mapping WIN32 Constant to description
 **/
static bool ParseArgs(int argc, wchar_t** argv, Arguments& out) {
    if (argc < 3)
        return false;

    for (auto i = 1; i < argc; ++i) {
        std::wstring a = argv[i];
        if (a == L"-p" && i + 1 < argc) {
            out.pid = std::wcstoul(argv[++i], nullptr, 10);
        } else if (a == L"-n" && i + 1 < argc) {
            out.name = argv[++i];
        } else if (a == L"--csv") {
            out.csv = true;
        } else if (a == L"--committed") {
            out.committedOnly = true;
        } else {
            return false;
        }
    }
    if (!out.pid.has_value() && !out.name.has_value())
        return false;

    return true;
}

/**
 Print pointer as hexadecimal
 **/
static std::wstring HexPtr(uintptr_t p) {
    std::wostringstream retval;
    retval << L"0x" << std::hex << std::setw(sizeof(uintptr_t) * 2)
        << std::setfill(L'0') << p;

    return retval.str();
}

/**
 Entry point
 **/
int wmain(int argc, wchar_t** argv) {
    auto pid = 0;
    Arguments args;
    CLEAR_SCREEN();

    if (!ParseArgs(argc, argv, args)) {
        PrintUsage();
        return 1;
    }

    if (args.pid.has_value()) {
        pid = *args.pid;
    } else {
        auto found = FindPidByName(*args.name);
        if (!found.has_value()) {
            std::wcerr << L"Process not found: " << *args.name << L"\n";
            return 2;
        }
        pid = *found;
    }

    SYSTEM_INFO si{};
    auto hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProc) {
        std::wcerr << L"OpenProcess failed. Try running as administrator. PID=" << pid << L"\n";
        return 3;
    }

    GetSystemInfo(&si);
    auto start = reinterpret_cast<uintptr_t>(si.lpMinimumApplicationAddress);
    auto end   = reinterpret_cast<uintptr_t>(si.lpMaximumApplicationAddress);

    if (args.csv) {
        std::wcout << L"Base,End,SizeBytes,State,Protect,Type,AllocBase\n";
    } else {
        std::wcout << L"PID " << pid << L" virtual memory layout\n";
        std::wcout << L"-------------------------------------------------------------------------------\n";
        std::wcout << L"Base                 End                  Size        State   Protect    Type    AllocBase\n";
    }

    auto addr = start;
    MEMORY_BASIC_INFORMATION mbi{};

    while (addr < end) {
        auto r = VirtualQueryEx(hProc, reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(mbi));

        if (r == 0) {
            addr += si.dwPageSize;
            continue;
        }

        auto state = mbi.State;

        if (args.committedOnly && state != MEM_COMMIT) {
            addr = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
            continue;
        }

        auto base = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
        auto regionEnd = base + mbi.RegionSize;
        auto baseStr = HexPtr(base);
        auto endStr  = HexPtr(regionEnd);
        auto allocBaseStr = mbi.AllocationBase ? HexPtr(reinterpret_cast<uintptr_t>(mbi.AllocationBase)) : L"N/A";
        auto stateStr = StateToStr(state);
        auto protStr  = ProtectToStr(mbi.Protect, state);
        auto typeStr  = TypeToStr(mbi.Type, state);

        if (args.csv) {
            std::wcout << baseStr << L"," << endStr << L"," << mbi.RegionSize << L","
                       << stateStr << L"," << protStr << L"," << typeStr << L"," << allocBaseStr << L"\n";
        } else {
            std::wcout << std::left
                       << std::setw(20) << baseStr << L"  "
                       << std::setw(20) << endStr  << L"  "
                       << std::setw(12) << mbi.RegionSize << L"  "
                       << std::setw(7)  << stateStr << L"  "
                       << std::setw(9)  << protStr  << L"  "
                       << std::setw(7)  << typeStr  << L"  "
                       << allocBaseStr << L"\n";
        }
        addr = regionEnd;
    }

    CloseHandle(hProc);

    return 0;
}