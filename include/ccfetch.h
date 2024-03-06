#ifndef CCFETCH_H
#define CCFETCH_H

#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>

using std::string;
using std::vector;
using std::runtime_error;
using std::ifstream;

string get_user();
string get_hostname(string path);
string get_distro(string path);
string get_hardware_platform();
string get_host(string path);
string get_kernel(string path);
string get_uptime(string path);
string get_memory(string path);
string get_shell(string path);
string get_de();
string get_wm();
string get_resolution(string path);
string get_theme();
string get_icons();
string get_cpu(string path);
int get_cpu_temp(string path);
bool cpu_temp_check(string path);
vector<string> get_gpu();
string get_packages();
string get_color(string distro);
ifstream get_logo(string distro);
int get_logo_width(ifstream& logo);
void print_help();
void print_logo();
void print_divider(char divider, int width);
void print_info(ifstream& logo, string label, string info, int width);
void ccfetch();

template<typename T>
void expect(const T& want, const T& got, const string& message) {
    if(want == got) {
        return;
    }
    std::cout << "Error: " << message << " (" << want << "), but got (" << got << ")\n";
    exit(EXIT_FAILURE);
}

class Command {
public:
    static Command exec(const string& command) {
        Command result = Command();
        FILE* pipe = popen(command.c_str(), "r");
        if(!pipe) {
            throw runtime_error("popen failed: \"" + command + "\"");
        }
        int c;
        while((c = fgetc(pipe)) != EOF) {
            if(c == '\n') {
                result.lines_ += 1;
            }
            result.output_ += c;
        }
        int n = pclose(pipe);
        result.exit_code_ = WEXITSTATUS(n);
        return result;
    }
    string get_output() {
        return output_;
    }
    int get_output_lines() {
        return lines_;
    }
    int get_exit_code() {
        return exit_code_;
    }
private:
    int exit_code_;
    string output_;
    int lines_;

    Command() : output_(), lines_(0) {}
};

class Path {
public:
    static Path of(const string& path) {
        return Path(std::filesystem::path(path), std::filesystem::status(path));
    }

    bool is_regular_file() {
        return std::filesystem::is_regular_file(status_);
    }
    
    bool is_executable() {
        if(status_.permissions() == std::filesystem::perms::unknown) {
            return false;
        }
        return (status_.permissions() & std::filesystem::perms::others_exec) != std::filesystem::perms::none;
    }
    
    bool is_directory() {
        return std::filesystem::is_directory(status_);
    }

    string to_string() {
        return path_.string();
    }
private:
    std::filesystem::path path_;    
    std::filesystem::file_status status_;
    Path(std::filesystem::path path, std::filesystem::file_status status) : path_(path), status_(status) {}
};

#endif // CCFETCH_H
