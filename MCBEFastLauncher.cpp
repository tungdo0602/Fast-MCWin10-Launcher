#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <sstream>
#include <vector>
#include <format>
#include <chrono>
#include <thread>

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::vector<std::string> split(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

std::string removeExtraSpaces(std::string str) {
    std::string nstr;
    for (int i = 0; i < str.length(); ) {
        if (str[i] == ' ') {
            if (i == 0 || i == str.length() - 1) {
                i++;
                continue;
            }
            while (str[i + 1] == ' ')
                i++;
        }
        nstr += str[i++];
    }
    return nstr;
}

std::string strip(std::string str){
    str.erase(0, str.find_first_not_of(" \t\n\r\f\v"));
    str.erase(str.find_last_not_of(" \t\n\r\f\v") + 1);
    return str;
}

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

std::string findPID(std::string processName, std::string packageName) {
    std::vector<std::string> raw = split(exec("tasklist /apps"), "=");
    std::string proc = strip(raw.back());
    std::vector<std::vector<std::string>> proclist;
    for (std::string i : split(proc, "\n")) {
        proclist.push_back(split(removeExtraSpaces(i), " "));
    }
    for (std::vector<std::string> p : proclist) {
        if ((p.back().find(packageName) != std::string::npos) && (p[0].find(processName) != std::string::npos)) {
            for (std::string i : p) {
                if (is_number(i)) {
                    return i;
                }
            }
        }
    }
    return "";
}

int main()
{
    using namespace std::this_thread;
    using namespace std::chrono_literals;
    int c = 0;
    std::cout << "Starting Minecraft...\n";
    system("explorer shell:appsFolder\\Microsoft.MinecraftUWP_8wekyb3d8bbwe!App");
    while (c < 5) {
        std::string pid = findPID("RuntimeBroker.exe", "Microsoft.MinecraftUWP");
        if (pid != "") {
            std::string cmd = "taskkill /F /PID " + pid;
            std::cout << exec(cmd.c_str());
            c += 1;
            if (c < 5){
                sleep_for(1s);
            }
        }
    }
    return 0;
}
