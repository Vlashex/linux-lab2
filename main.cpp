#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>

#include <unistd.h>
#include <sys/wait.h>

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

static bool handle_env(const std::string& input) {
    if (input.rfind("\\e", 0) != 0) {
        return false;
    }

    std::size_t pos = 2;
    while (pos < input.size() && input[pos] == ' ') {
        ++pos;
    }

    if (pos >= input.size()) {
        std::cout << "Usage: \\e $VARIABLE" << '\n';
        return true;
    }

    std::string variable_name = input.substr(pos);
    if (!variable_name.empty() && variable_name.front() == '$') {
        variable_name.erase(variable_name.begin());
    }

    const char* env_value = std::getenv(variable_name.c_str());
    if (!env_value) {
        std::cout << "Environment variable '" << variable_name
                  << "' not found" << '\n';
        return true;
    }

    const std::string value(env_value);
    std::size_t start = 0;
    std::size_t end = value.find(':');

    while (end != std::string::npos) {
        std::cout << value.substr(start, end - start) << '\n';
        start = end + 1;
        end = value.find(':', start);
    }

    std::cout << value.substr(start) << '\n';
    return true;
}

static void execute_external(const std::string& input) {
    std::vector<std::string> args;
    std::stringstream stream(input);
    std::string token;

    while (stream >> token) {
        args.push_back(token);
    }

    if (args.empty()) {
        return;
    }

    pid_t pid = ::fork();
    if (pid == 0) {
        std::vector<char*> argv;
        argv.reserve(args.size() + 1);

        for (auto& argument : args) {
            argv.push_back(const_cast<char*>(argument.c_str()));
        }
        argv.push_back(nullptr);

        ::execvp(argv[0], argv.data());

        std::cout << input << ": command not found\n";
        ::_exit(1);
    }

    if (pid > 0) {
        int status = 0;
        ::waitpid(pid, &status, 0);
    } else {
        std::cerr << "Failed to create process" << std::endl;
    }
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

        if (input.empty()) {
            std::cerr << "$ ";
            continue;
        }

        if (handle_echo(input)) {
            std::cerr << "$ ";
            continue;
        }

        if (handle_env(input)) {
            std::cerr << "$ ";
            continue;
        }

        execute_external(input);
        std::cerr << "$ ";
    }

    return 0;
}
