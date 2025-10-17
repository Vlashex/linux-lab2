#include <iostream>
#include <string>

int main() {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::string input;

    std::cerr << "$ ";
    while (std::getline(std::cin, input)) {
        if (input == "\\q") {
            break;
        }

        std::cout << input << '\n';
        std::cerr << "$ ";
    }

    return 0;
}
