// ===================== SYSTEM MONITOR =====================
// Author: Upendra Singh (WSL & Linux Fixed Version)
// Build: g++ -std=c++17 system_monitor.cpp -o system_monitor
// Run:   ./system_monitor
// ==========================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <unistd.h>
#include <dirent.h>
#include <iomanip>
#include <algorithm>
#include <sys/types.h>
#include <signal.h>
#include <chrono>
#include <termios.h>
#include <fcntl.h>

struct ProcessInfo {
    int pid;
    std::string name;
    float cpuUsage;
    float memUsage;
    unsigned long long lastCpuTime;
};

// -----------------------------------------------------------
// Non-blocking keyboard input detection
// -----------------------------------------------------------
int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

// -----------------------------------------------------------
// CPU and Memory Usage Functions
// -----------------------------------------------------------
struct CPUData {
    unsigned long long total;
    unsigned long long idle;
};

CPUData readCPUStat() {
    std::ifstream file("/proc/stat");
    std::string cpu;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    file >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    return {user + nice + system + idle + iowait + irq + softirq + steal, idle + iowait};
}

float getCPUUsagePercent(const CPUData& prev, const CPUData& curr) {
    unsigned long long diffTotal = curr.total - prev.total;
    unsigned long long diffIdle = curr.idle - prev.idle;
    if (diffTotal == 0) return 0.0f;
    return (float)(diffTotal - diffIdle) / diffTotal * 100.0f;
}

float getMemoryUsagePercent() {
    std::ifstream file("/proc/meminfo");
    std::string label;
    long totalMem = 0, freeMem = 0, buffers = 0, cached = 0;
    std::string tmp;

    while (file >> label) {
        if (label == "MemTotal:") file >> totalMem >> tmp;
        else if (label == "MemFree:") file >> freeMem >> tmp;
        else if (label == "Buffers:") file >> buffers >> tmp;
        else if (label == "Cached:") file >> cached >> tmp;
    }

    long used = totalMem - freeMem - buffers - cached;
    return (float)used / totalMem * 100.0f;
}

// -----------------------------------------------------------
// Process Info Collector (with CPU delta over time)
// -----------------------------------------------------------
unsigned long long getProcessCpuTime(int pid) {
    std::ifstream statFile("/proc/" + std::to_string(pid) + "/stat");
    if (!statFile) return 0;
    std::string temp;
    std::getline(statFile, temp);
    std::istringstream iss(temp);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) tokens.push_back(token);
    if (tokens.size() < 17) return 0;
    unsigned long long utime = std::stoull(tokens[13]);
    unsigned long long stime = std::stoull(tokens[14]);
    return utime + stime;
}

std::vector<ProcessInfo> getProcesses(const std::vector<ProcessInfo>& prevProcs, unsigned long long totalDiff, long ticksPerSec) {
    std::vector<ProcessInfo> processes;
    DIR* dir = opendir("/proc");
    if (!dir) return processes;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (!isdigit(entry->d_name[0])) continue;
        int pid = std::stoi(entry->d_name);

        std::ifstream statFile("/proc/" + std::to_string(pid) + "/stat");
        if (!statFile) continue;

        std::string temp;
        std::getline(statFile, temp);
        std::istringstream iss(temp);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) tokens.push_back(token);
        if (tokens.size() < 24) continue;

        std::string comm = tokens[1];
        unsigned long long totalTime = std::stoull(tokens[13]) + std::stoull(tokens[14]);
        long rss = std::stol(tokens[23]);
        long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024;
        float memUsage = rss * page_size_kb / 1024.0f; // MB

        float cpuUsage = 0.0f;
        for (const auto& p : prevProcs) {
            if (p.pid == pid) {
                unsigned long long diff = totalTime - p.lastCpuTime;
                cpuUsage = (float)diff / (totalDiff * ticksPerSec) * 100.0f;
                break;
            }
        }

        processes.push_back({pid, comm, cpuUsage, memUsage, totalTime});
    }
    closedir(dir);
    return processes;
}

// -----------------------------------------------------------
// Display Function
// -----------------------------------------------------------
void display(const std::vector<ProcessInfo>& processes, float cpu, float mem) {
    system("clear");
    std::cout << "===================== SYSTEM MONITOR =====================\n";
    std::cout << "CPU Usage: " << std::fixed << std::setprecision(2) << cpu << "%  "
              << "Memory Usage: " << mem << "%\n";
    std::cout << "----------------------------------------------------------\n";
    std::cout << std::left << std::setw(8) << "PID"
              << std::setw(25) << "PROCESS"
              << std::setw(12) << "CPU(%)"
              << std::setw(12) << "MEM(MB)"
              << "\n";
    std::cout << "----------------------------------------------------------\n";
    for (const auto& p : processes) {
        std::cout << std::left << std::setw(8) << p.pid
                  << std::setw(25) << p.name.substr(0, 24)
                  << std::setw(12) << std::fixed << std::setprecision(2) << p.cpuUsage
                  << std::setw(12) << std::fixed << std::setprecision(2) << p.memUsage
                  << "\n";
    }
    std::cout << "----------------------------------------------------------\n";
    std::cout << "Press 'k' to kill process, 'q' to quit.\n";
}

// -----------------------------------------------------------
// Main Loop
// -----------------------------------------------------------
int main() {
    long ticksPerSec = sysconf(_SC_CLK_TCK);
    CPUData prevCPU = readCPUStat();
    std::vector<ProcessInfo> prevProcs = getProcesses({}, 1, ticksPerSec);

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        CPUData currCPU = readCPUStat();
        float cpuUsage = getCPUUsagePercent(prevCPU, currCPU);
        unsigned long long totalDiff = currCPU.total - prevCPU.total;
        prevCPU = currCPU;

        float memUsage = getMemoryUsagePercent();
        auto processes = getProcesses(prevProcs, totalDiff, ticksPerSec);
        prevProcs = processes;

        std::sort(processes.begin(), processes.end(),
                  [](const ProcessInfo& a, const ProcessInfo& b) {
                      return a.cpuUsage > b.cpuUsage;
                  });

        display(processes, cpuUsage, memUsage);

        if (kbhit()) {
            char ch = getchar();
            if (ch == 'q') {
                std::cout << "Exiting monitor...\n";
                break;
            } else if (ch == 'k') {
                int pid;
                std::cout << "Enter PID to kill: ";
                std::cin >> pid;
                if (kill(pid, SIGTERM) == 0)
                    std::cout << "Process " << pid << " terminated.\n";
                else
                    std::cout << "Failed to kill process (try sudo).\n";
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        }
    }
    return 0;
}
