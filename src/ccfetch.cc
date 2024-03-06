#include "ccfetch.h"

#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unistd.h>
#include <iomanip>

using std::string;
using std::vector;
using std::ios;
using std::ifstream;
using std::ofstream;

struct {
    string black = "\033[30m";
    string cyan = "\033[36m";
    string boldcyan = "\033[1m\033[36m";
    string reset = "\033[0m";
    string white = "\033[37m";
    string boldwhite = "\033[1m\033[37m";
    string boldgreen = "\033[1m\033[32m";
    string red = "\033[31m";
} colors;

string get_user() {
    return getenv("USER");
}

string get_hostname(string path) {
    ifstream in_stream;
    in_stream.open(path, ios::in);
    string hostname;
    getline(in_stream, hostname);
    in_stream.close();
    return hostname;
}

string get_distro(string path) {
    ifstream in_stream;
    in_stream.open(path, ios::in);
    string line, sub;
    while(in_stream) {
        getline(in_stream, line);
        sub = line.substr(0, 13);
        if(sub == "PRETTY_NAME=\"") {
            break;
        }
    }
    line = line.substr(line.find("\"") + 1);
    line = line.substr(0, line.find("\""));
    in_stream.close();
    return line;
}

string get_hardware_platform() {
    string str = Command::exec("uname -m").get_output();
    str = str.substr(0, str.find("\n"));
    return str;
}

string get_host(string path) {
    ifstream in_stream;
    in_stream.open((path + "product_name"), ios::in);

    string str;
    getline(in_stream, str);
    in_stream.close();

    string host;
    host += str;

    in_stream.open((path + "product_version"), ios::in);
    getline(in_stream, str);
    in_stream.close();

    host = host + " " + str;

    return host;
}

string get_kernel(string path) {
    ifstream in_stream;
    in_stream.open(path, ios::in);
    string kernel;
    getline(in_stream, kernel);
    return kernel;
}

string get_uptime(string path) {
    ifstream in_stream;
    in_stream.open(path, ios::in);
    string time;
    getline(in_stream, time);
    time = time.substr(0, time.find(" "));

    int minutes = stoi(time) / 60;
    int hours = minutes / 60;
    int days = hours / 24;

    string time_str;
    if(hours == 0) {
        time_str = std::to_string(minutes % 60) + " minutes";
    } else if(days == 0) {
        time_str = std::to_string(hours % 24) + " hours, " + std::to_string(minutes % 60) + " minutes";
    } else {
        time_str = std::to_string(days) + " days, " + std::to_string(hours % 24) + " hours, " + std::to_string(minutes % 60) + " minutes";
    }

    return time_str;
}

string get_memory(string path) {
    ifstream in_stream;
    in_stream.open(path, ios::in);
    string line, sub, shmem;
    string total, free;
    while(in_stream) {
        getline(in_stream, line);
        sub = line.substr(0, line.find(":"));
        if(sub == "MemTotal") {
            total = line;
        }
        if(sub == "MemAvailable") {
            free = line;
        }
        if(sub == "Buffers") {
            shmem = line;
            break;
        }
    }
    size_t i;
    for(i = 0; i < total.size(); ++i) {
        if(isdigit(total[i])) {
            break;
        }
    }
    total = total.substr(i);
    total = total.substr(0, total.find(" "));
    for(i = 0; i < free.size(); ++i) {
        if(isdigit(free[i])) {
            break;
        }
    }
    free = free.substr(i);
    free = free.substr(0, free.find(" "));

    for(i = 0; i < shmem.size(); ++i) {
        if(isdigit(shmem[i])) {
            break;
        }
    }

    shmem = shmem.substr(i);
    shmem = shmem.substr(0, shmem.find(" "));

    int mem_total = stoi(total);
    int mem_free = stoi(free);
    int mem_avail = (mem_total - mem_free) - stoi(shmem);
    
    string ram = std::to_string(mem_avail / 1024) + "MiB / " + std::to_string(mem_total / 1024) + "MiB";
    
    return ram;
}

string get_packages() {
    string package;
    if(Path::of("/bin/pacman").is_executable()) {
        auto command = Command::exec("pacman -Q");
        package += std::to_string(command.get_output_lines());
    }
    return package;
}

string get_shell(string path) {
    ifstream in_stream;
    in_stream.open(path, ios::in);
    string line, sub;
    while(in_stream) {
        getline(in_stream, line);
        sub = line.substr(0 , line.find(":"));
        if(sub == get_user()) {
            break;
        }
    }
    reverse(line.begin(), line.end());
    line = line.substr(0, line.find("/"));
    reverse(line.begin(), line.end());
    return line;
}

string get_resolution(string path) {
    ifstream in_stream;
    in_stream.open(path, ios::in);
    string resolution;
    getline(in_stream, resolution);
    resolution = resolution.substr(2);
    resolution = resolution.substr(0, resolution.find("p"));
    return resolution;
}

string get_de() {
    string de;

    if(getenv("XDG_CURRENT_DESKTOP") != nullptr) {
        de = getenv("XDG_CURRENT_DESKTOP");
    }

    if(de.find("GNOME") != string::npos) {
        de = "GNOME";
    }

    return de;
}

string get_wm() {
    string wm;
    //string wm = Command::exec("ps -e | grep -m 1 -o -F -e arcan -e asc -e clayland -e dwc -e fireplace -e gnome-shell"
    //                      " -e greenfield -e grefsen -e hikari -e kwin -e lipstick -e maynard -e mazecompositor"
    //                      " -e motorcar -e orbital -e orbment -e perceptia -e river -e rustland -e sway -e ulubis"
    //                      " -e velox -e wavy -e way-cooler -e wayfire -e wayhouse -e westeros -e westford -e weston"
    //                      " -e i3 -e dwm -e openbox -e worm");

    //while(wm.find('\n') != string::npos) {
    //    wm.erase(wm.find('\n'), wm.find('\n') + 1);
    //}

    //if(wm.find("gnome-shell") != string::npos) {
    //    wm = "Mutter";
    //}

    return wm;
}

string get_cpu(string path) {
    ifstream in_stream;
    in_stream.open(path, ios::in);
    string cpu, line, sub;
    while(in_stream) {
        getline(in_stream, line);
        sub = line.substr(0, 10);
        if(sub == "model name") {
            break;
        }
    }
    cpu = line.substr(line.find(":") + 2);
    return cpu;
}

vector<string> get_gpu() {
    vector<string> gpus;
    auto command = Command::exec("lspci | grep -E  \"VGA|3D|Display\"");
    string gpu = command.get_output();
    int temp = 0, k = 0;
    for(size_t i = 0; i < gpu.size(); ++i) {
        if(gpu[i] == '\n') {
            gpus.push_back(gpu.substr(temp, i - temp));
            gpus[k] = gpus[k].substr(gpus[k].find(": ") + 2);
            gpus[k] = gpus[k].substr(0, gpus[k].find(" ("));
            temp = i + 1;
            ++k;
        }
    }
    return gpus;
}

ifstream get_logo(string distro) {
    string path = "ascii/default.ascii";
    if(distro == "Arch Linux") {
        path = "ascii/arch.ascii";
    }
    ifstream in_stream(path);
    return in_stream;
}

int get_logo_width(ifstream& logo) {
    string str;
    getline(logo, str);
    logo.clear();
    logo.seekg(0);
    return str.length();
}

void print_info(ifstream& logo, string label, string info, string color, int width) {
    string logo_str;
    if(getline(logo, logo_str)) {
        std::cout << color << logo_str << "   " << label << colors.reset << info << "\n";
    } else {
        string spacer = string(width, ' ');
        std::cout << spacer << color << "   " << label << colors.reset << info << "\n";
    }
}

void print_divider(char divider, int width) {
    for(int i = 0; i < width; ++i) {
        std::cout << divider;
    }
    std::cout << "\n";
}

string get_color(string distro) {
    if(distro == "Arch Linux") {
        return colors.boldcyan;
    }
    return colors.white;
}

void ccfetch() {
    string distro = get_distro("/etc/os-release");
    ifstream logo = get_logo(distro);
    int width = get_logo_width(logo);
    string user = get_user();
    string hostname = get_hostname("/etc/hostname");
    string platform = get_hardware_platform();
    string color = get_color(distro);

    string username = user + "@" + hostname;
    print_info(logo, username, "", color, width);

    print_info(logo, "", string(15, '-'), color, width);

    string os = distro + " " + platform;
    print_info(logo, "OS: ", os, color, width);

    string host = get_host("/sys/devices/virtual/dmi/id/");
    print_info(logo, "Host: ", host, color, width);

    string kernel = get_kernel("/proc/sys/kernel/osrelease");
    print_info(logo, "Kernel: ", kernel, color, width);

    string uptime = get_uptime("/proc/uptime");
    print_info(logo, "Uptime: ", uptime, color, width);

    string packages = get_packages();
    print_info(logo, "Packages: ", packages, color, width);

    string shell = get_shell("/etc/passwd");
    print_info(logo, "Shell: ", shell, color, width);

    string resolution = get_resolution("/sys/class/graphics/fb0/modes");
    print_info(logo, "Resolution: ", resolution, color, width);

    string cpu = get_cpu("/proc/cpuinfo");
    print_info(logo, "CPU: ", cpu, color, width);

    vector<string> gpus = get_gpu();
    for(auto gpu : gpus) {
        print_info(logo, "GPU: ", gpu, color, width);
    }

    string memory = get_memory("/proc/meminfo");
    print_info(logo, "Memory: ", memory, color, width);
    
    string logo_str;
    while(getline(logo, logo_str)) {
        std::cout << color << logo_str << colors.reset << '\n';
    }
}
