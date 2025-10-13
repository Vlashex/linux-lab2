#include <iostream>
#include <string>

int main() {
    std::cout << std::unitbuf;

    std::string input;
    while (std::getline(std::cin, input)) {
        std::cout << input << '\n';
    }

    return 0;
}
