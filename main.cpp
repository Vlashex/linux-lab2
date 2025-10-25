#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>

static bool handle_echo(const std::string& input) {
    if (input.rfind("echo", 0) != 0) {
        return false;
    }

    std::string payload = input.substr(4);

    while (!payload.empty() && payload.front() == ' ') {
        payload.erase(payload.begin());
    }

    if (!payload.empty()) {
        const char first = payload.front();
        const char last  = payload.back();

        if ((first == '"' && last == '"') ||
            (first == '\'' && last == '\'')) {
            if (payload.size() >= 2) {
                payload = payload.substr(1, payload.size() - 2);
            } else {
                payload.clear();
            }
        }
    }

    std::cout << payload << '\n';
    return true;
}

int main() {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    const char* home = std::getenv("HOME");
    std::string history_path;
    if (home) {
        history_path = std::string(home) + "/.kubsh_history";
    } else {
        history_path = ".kubsh_history";
    }

    std::ofstream history_stream(history_path, std::ios::app);
    std::vector<std::string> history;

    std::string input;
    std::cerr << "$ ";
    while (std::getline(std::cin, input)) {
        history.push_back(input);

        if (history_stream.is_open()) {
            history_stream << input << '\n';
            history_stream.flush();
        }

        if (input == "\\q") {
            break;
        }

        if (handle_echo(input)) {
            std::cerr << "$ ";
            continue;
        }

        std::cout << input << '\n';
        std::cerr << "$ ";
    }

    return 0;
}
