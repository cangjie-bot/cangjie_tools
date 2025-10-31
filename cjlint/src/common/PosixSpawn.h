#ifndef COMMAND_EXECUTOR_H
#define COMMAND_EXECUTOR_H

#include <string>
#include <stdexcept>
#include <cstring>

//平台检测
#ifdef _WIN32
#include <windows.h>
#define PLATFORM_WINDOWS
#elif __linux__
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
extern char **environ;
#define PLATFORM_LINUX
#elif __APPLE__
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
extern char **environ;
#define PLATFORM_MACOS
#endif

class CommandExecutor {
public:
    static int executor(const std::string& command) {
#ifdef _WIN32
        return execute_on_windows(command);
#else
        return execute_on_posix(command);
#endif
    }

private:
#ifdef PLATFORM_WINDOWS
    static int execute_on_windows(const std::string& command) {
        std:string cmd_line = "cmd /c " + command;

        STARTUPINFO si{};
        si.cb = sizeof(STARTUPINFO);
        PROCESS_INFORMATION pi{};

        BOOL success = CreateProcessA(
            nullptr,
            const_cast<char*>(cmd_line.c_str()),
            nullptr,
            nullptr,
            FALSE,
            0,
            nullptr,
            nullptr,
            &si,
            &pi
        );

        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exit_code;
        GetExitCodeProcess(pi.hProcess, &exit_code);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return static_cast<int>(exit_code);
    }
#else
    static int executor_on_posix(const std::string& command) {
        pid_t pid;

        char* argv[] = {
            const_cast<char*>(command.c_str()),
            nullptr
        };

        int spawn_result = posix_spawnp(&pid, argv[0], nullptr, nullptr, argv, environ);
    }
#endif
};

#endif