#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>

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

        std::cout << input << '\n';
        std::cerr << "$ ";
    }

    return 0;
}
