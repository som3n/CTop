#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <vector>
#include <pwd.h>
#include <dirent.h>
#include <ctype.h>
#include <algorithm>
#include <iomanip>
#include <sys/sysinfo.h>

using namespace std;

struct ProcessInfo {
    int pid;
    string user;
    double cpu;       // CPU usage in percentage
    double memPercent; // Memory usage as a percentage of total memory
    double virtGB;    // Virtual memory size in GB
    double uptime;    // Uptime in seconds
    string uptimeStr; // Uptime as hh:mm:ss
    string command;
};

string getUserName(uid_t uid) {
    struct passwd *pwd;
    if ((pwd = getpwuid(uid)) != NULL) {
        return pwd->pw_name;
    } else {
        return "unknown";
    }
}

bool isAllDigits(const string& str) {
    for (char c : str) {
        if (!isdigit(c)) {
            return false;
        }
    }
    return true;
}

string formatUptime(double uptime) {
    int hours = static_cast<int>(uptime) / 3600;
    int minutes = (static_cast<int>(uptime) % 3600) / 60;
    int seconds = static_cast<int>(uptime) % 60;

    stringstream ss;
    ss << setfill('0') << setw(2) << hours << ":"
       << setw(2) << minutes << ":"
       << setw(2) << seconds;

    return ss.str();
}

vector<ProcessInfo> getProcesses(double totalMemory, double totalCpuTime) {
    vector<ProcessInfo> processes;
    DIR *dir;
    struct dirent *entry;
    if ((dir = opendir("/proc")) == NULL) {
        cerr << "Cannot open /proc directory" << endl;
        return processes;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            string dirName(entry->d_name);
            if (isAllDigits(dirName)) {
                int pid = stoi(dirName);

                // Read process command and details
                stringstream procPath;
                procPath << "/proc/" << pid << "/stat";
                ifstream statFile(procPath.str());

                if (statFile.is_open()) {
                    string comm, state;
                    int ppid, pgrp, session, tty_nr, tpgid, flags;
                    unsigned long utime, stime, cutime, cstime;
                    unsigned long starttime;
                    statFile >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags
                             >> utime >> stime >> cutime >> cstime >> starttime;

                    // Get UID, memory, and uptime
                    stringstream statusPath;
                    statusPath << "/proc/" << pid << "/status";
                    ifstream statusFile(statusPath.str());

                    if (statusFile.is_open()) {
                        string line;
                        uid_t uid = -1;
                        double memKB = 0.0;
                        double virtKB = 0.0;
                        double uptime = 0.0;

                        while (getline(statusFile, line)) {
                            if (line.find("Uid:") == 0) {
                                stringstream uidStream(line.substr(4));
                                uidStream >> uid;
                            }
                            if (line.find("VmRSS:") == 0) {  // Resident Set Size (RSS) memory
                                stringstream memStream(line.substr(6));
                                memStream >> memKB;
                            }
                            if (line.find("VmSize:") == 0) {  // Virtual Memory Size
                                stringstream virtStream(line.substr(7));
                                virtStream >> virtKB;
                            }
                        }

                        // Convert memory and virtual memory from KB to GB
                        double memMB = memKB / 1024.0;
                        double virtGB = (virtKB / 1024.0) / 1024.0; // Convert MB to GB

                        // Calculate memory percentage
                        double memPercent = (memMB / totalMemory) * 100.0;

                        // Calculate CPU usage
                        unsigned long totalTime = utime + stime;
                        double cpuUsage = (totalTime / (double)totalCpuTime) * 100.0; // CPU usage percentage

                        // Calculate uptime
                        uptime = (utime + stime) / sysconf(_SC_CLK_TCK); // in seconds

                        ProcessInfo proc;
                        proc.pid = pid;
                        proc.command = comm.substr(1, comm.size() - 2);  // Remove parentheses
                        if (proc.command.length() > 50) {
                            proc.command = proc.command.substr(0, 50) + "..."; // Truncate to 50 characters
                        }
                        proc.cpu = cpuUsage; // CPU usage percentage
                        proc.memPercent = memPercent; // Memory usage as percentage
                        proc.virtGB = virtGB; // Virtual memory size in GB
                        proc.uptime = uptime; // Uptime in seconds
                        proc.uptimeStr = formatUptime(uptime); // Format uptime
                        proc.user = getUserName(uid);

                        processes.push_back(proc);
                    }
                }
            }
        }
    }

    closedir(dir);

    // Sort processes by memory percentage in descending order
    sort(processes.begin(), processes.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
        return a.memPercent > b.memPercent;
    });

    return processes;
}

void displayProcesses(const vector<ProcessInfo>& processes) {
    for (const auto& proc : processes) {
        // Print each process info with improved formatting
        cout << proc.pid << " "
             << proc.user << " "
             << fixed << setprecision(2) << proc.cpu/10 << " "  // Display CPU usage with percentage symbol
             << fixed << setprecision(2) << proc.memPercent << " "  // Display memory usage with percentage symbol
             << fixed << setprecision(2) << proc.virtGB << " "  // Display virtual memory in GB
             << proc.uptimeStr << " " // Print uptime as hh:mm:ss
             << setw(50) << left << proc.command << endl;
    }
}

int main() {
    // Get total system memory and total CPU time
    struct sysinfo info;
    sysinfo(&info);
    double totalMemory = info.totalram / (1024.0 * 1024.0);  // Convert to MB

    // Get total CPU time
    stringstream cpuPath;
    cpuPath << "/proc/stat";
    ifstream cpuFile(cpuPath.str());

    double totalCpuTime = 0.0;
    if (cpuFile.is_open()) {
        string line;
        getline(cpuFile, line);
        stringstream cpuStream(line);
        string cpuLabel;
        unsigned long user, nice, system, idle;
        cpuStream >> cpuLabel >> user >> nice >> system >> idle;
        totalCpuTime = user + nice + system + idle; // Total CPU time
    }

    vector<ProcessInfo> processes = getProcesses(totalMemory, totalCpuTime);
    displayProcesses(processes);
    return 0;
}
