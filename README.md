🖥️ System Monitor Tool (Assignment 3 – Linux/WSL Fixed Version)
Author: Upendra Singh
Language: C++ (C++17 Standard)
📘 Project Overview

The System Monitor Tool is a console-based real-time monitoring utility built in C++ for Linux and WSL environments.
It provides detailed system insights such as CPU usage, memory utilization, and active processes along with interactive process management.
This project replicates the functionality of the Linux top command in a simplified and educational form.

🎯 Objective

To build a real-time system monitoring application that displays live information about:

CPU usage percentage

Memory consumption

List of active processes with PID, CPU%, and memory (MB)

Process termination via keyboard commands

⚙️ Features

🔹 Real-Time CPU and Memory Monitoring

🔹 Dynamic Process Listing with PID, process name, CPU%, and memory usage

🔹 Sorting by CPU Usage

🔹 Process Termination (Kill Function)

🔹 Non-blocking Keyboard Interaction

Press ‘k’ → Kill a process

Press ‘q’ → Quit the monitor

🔹 Clean Terminal UI with live auto-refresh every second

🧠 Technical Details
Component	Description
Language	C++ (C++17)
Platform	Linux / WSL
Build Tool	g++ Compiler
System Interface	/proc Filesystem
Libraries Used	<dirent.h>, <unistd.h>, <termios.h>, <chrono>, <thread>, <signal.h>, <fstream>, <sstream>
🛠️ Build and Run Instructions
Build Command
g++ -std=c++17 system_monitor.cpp -o system_monitor

Run Command
./system_monitor

📊 Sample Output
===================== SYSTEM MONITOR =====================
CPU Usage: 14.56%   Memory Usage: 47.12%
----------------------------------------------------------
PID     PROCESS                  CPU(%)     MEM(MB)
----------------------------------------------------------
1234    firefox                  22.34      512.00
5678    code                     15.89      378.00
9821    systemd                   1.23       40.00
----------------------------------------------------------
Press 'k' to kill process, 'q' to quit.

🔍 Working Mechanism

Reads system statistics from /proc/stat and /proc/meminfo.

Extracts process information from /proc/[pid]/stat files.

Calculates CPU usage deltas and memory utilization.

Displays all running processes sorted by CPU usage.

Listens for keyboard input to kill processes or exit gracefully.

⚡ Implementation Timeline
Day	Task
Day 1	Designed UI and fetched system data using system calls
Day 2	Displayed process list with CPU and memory usage
Day 3	Implemented process sorting and CPU delta tracking
Day 4	Added kill functionality using kill(pid, SIGTERM)
Day 5	Integrated live refresh and keyboard event handling
🧩 Challenges Faced

Accurate computation of CPU deltas from /proc/stat

Handling permission errors when killing system processes

Implementing non-blocking input for real-time interaction

Maintaining smooth display refresh without flickering

🧾 Learning Outcomes

Proficiency in Linux system programming

Understanding of /proc filesystem and process management

Hands-on with signal handling and I/O control

Experience building a real-time terminal application in C++

✅ Conclusion

The System Monitor Tool is a lightweight and functional C++ utility that provides real-time system performance data and process control through an intuitive console interface.
It demonstrates a strong grasp of system-level programming, resource tracking, and terminal interactivity, serving as a solid foundation for advanced Linux utilities.

📂 Repository Structure
📁 System-Monitor-Tool
├── system_monitor.cpp
├── README.md
└── System_Monitor_Tool_Report_Upendra_Singh.pdf

🏷️ Version

WSL & Linux Fixed Version – Compatible with both Ubuntu (Linux) and WSL environments.
