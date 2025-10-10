#include <iostream>
#include <string>

int main() {
    std::cout << std::unitbuf;

    std::string input;
    if (std::getline(std::cin, input)) {
        std::cout << input << '\n';
    }

    return 0;
}
