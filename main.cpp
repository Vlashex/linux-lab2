#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <csignal>
#include <cstring>
#include <cstdint>
#include <cerrno>
#include <cstdlib>

#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

class ShellSignalManager {
public:
    static void install_sighup_handler() {
        struct sigaction action;
        std::memset(&action, 0, sizeof(action));
        action.sa_handler = &ShellSignalManager::sighup_handler;
        action.sa_flags = SA_RESTART;
        sigaction(SIGHUP, &action, nullptr);
    }

    static bool is_sighup_received() {
        return sighup_flag_ != 0;
    }

    static void clear_sighup() {
        sighup_flag_ = 0;
    }

private:
    static void sighup_handler(int) {
        sighup_flag_ = 1;
    }

    static volatile sig_atomic_t sighup_flag_;
};

volatile sig_atomic_t ShellSignalManager::sighup_flag_ = 0;

class ShellCommandExecutor {
public:
    static void execute_debug(const std::string& input) {
        std::string payload = input.substr(5);

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
    }

    static void print_environment_variable(const std::string& input) {
        const std::size_t command_pos = input.find("\\e");
        const std::size_t var_pos = command_pos + 3;

        if (var_pos < input.length()) {
            std::string variable_name = input.substr(var_pos);

            if (!variable_name.empty() && variable_name.front() == '$') {
                variable_name.erase(variable_name.begin());
            }

            const char* env_value = std::getenv(variable_name.c_str());
            if (env_value != nullptr) {
                const std::string value(env_value);
                std::size_t start = 0;
                std::size_t end = value.find(':');

                while (end != std::string::npos) {
                    std::cout << value.substr(start, end - start) << '\n';
                    start = end + 1;
                    end = value.find(':', start);
                }

                std::cout << value.substr(start) << '\n';
            } else {
                std::cout << "Environment variable '" << variable_name
                          << "' not found" << '\n';
            }
        } else {
            std::cout << "Usage: \\e $VARIABLE" << '\n';
        }
    }

    static void execute_external(const std::string& input) {
        const pid_t pid = ::fork();

        if (pid == 0) {
            std::vector<std::string> args;
            std::stringstream stream(input);
            std::string token;

            while (stream >> token) {
                args.push_back(token);
            }

            std::vector<char*> argv;
            argv.reserve(args.size() + 1);

            for (auto& argument : args) {
                argv.push_back(const_cast<char*>(argument.c_str()));
            }
            argv.push_back(nullptr);

            ::execvp(argv[0], argv.data());

            std::cout << input << ": command not found\n";
            std::_Exit(1);
        }

        if (pid > 0) {
            int status = 0;
            ::waitpid(pid, &status, 0);
        } else {
            std::cerr << "Failed to create process" << '\n';
        }
    }
};

class InteractiveShell {
public:
    InteractiveShell()
        : history_file_path_("kubsh_history.txt"),
          history_stream_(history_file_path_, std::ios::app) {}

    void run() {
        std::cerr << "$ ";

        std::string input;
        while (std::getline(std::cin, input)) {
            if (ShellSignalManager::is_sighup_received()) {
                std::cout << "Configuration reloaded" << std::endl;
                ShellSignalManager::clear_sighup();
                std::cerr << "$ ";
                continue;
            }

            trim_leading_spaces(input);
            append_to_history(input);

            if (input == "\\q") {
                break;
            }

            if (input.empty()) {
                std::cerr << "$ ";
                continue;
            }

            if (input.find("debug") == 0) {
                ShellCommandExecutor::execute_debug(input);
            } else if (input.find("\\e") == 0) {
                ShellCommandExecutor::print_environment_variable(input);
            } else {
                ShellCommandExecutor::execute_external(input);
            }

            std::cerr << "$ ";
        }
    }

private:
    static void trim_leading_spaces(std::string& input) {
        while (!input.empty() && input.front() == ' ') {
            input.erase(input.begin());
        }
    }

    void append_to_history(const std::string& input) {
        if (history_stream_.is_open()) {
            history_stream_ << '$' << input << '\n';
            history_stream_.flush();
        }
    }

    std::string   history_file_path_;
    std::ofstream history_stream_;
};

int main() {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    ShellSignalManager::install_sighup_handler();

    InteractiveShell shell;
    shell.run();

    return 0;
}
