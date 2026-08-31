#ifndef TAN_H
#define TAN_H

#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pwd.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#else
#include <spawn.h>
extern char **environ;
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

class TAN {
private:
    static std::string terminalPath;
    static bool initialized;
    static std::string configDir;

    static void ensureConfigDir() {
        const char* home = NULL;
#ifdef _WIN32
        home = getenv("USERPROFILE");
        if (!home) home = getenv("HOME");
        if (!home) home = "C:\\";
#else
        home = getenv("HOME");
        if (!home) {
            struct passwd* pw = getpwuid(getuid());
            if (pw) home = pw->pw_dir;
        }
        if (!home) home = "/tmp";
#endif
        configDir = std::string(home) + "/.config/error.os";
#ifdef _WIN32
        mkdir(configDir.c_str());
#else
        mkdir(configDir.c_str(), 0755);
#endif
    }

    static std::string getSyslinkPath() {
        ensureConfigDir();
        return configDir + "/tan";
    }

    static bool accessible(const std::string& path, bool needExec) {
#ifdef _WIN32
        (void)needExec;
        return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
#else
        return access(path.c_str(), needExec ? X_OK : F_OK) == 0;
#endif
    }

    static std::string findInPath(const std::string& name) {
#ifdef _WIN32
        char path[MAX_PATH];
        if (SearchPathA(NULL, name.c_str(), ".exe", MAX_PATH, path, NULL))
            return std::string(path);
        if (SearchPathA(NULL, name.c_str(), ".cmd", MAX_PATH, path, NULL))
            return std::string(path);
        if (SearchPathA(NULL, name.c_str(), ".bat", MAX_PATH, path, NULL))
            return std::string(path);
#else
        const char* pathEnv = getenv("PATH");
        if (!pathEnv) return "";
        std::string pathStr(pathEnv);
        size_t start = 0;
        while (start <= pathStr.size()) {
            size_t end = pathStr.find(':', start);
            std::string dir = (end == std::string::npos)
                                  ? pathStr.substr(start)
                                  : pathStr.substr(start, end - start);
            if (!dir.empty()) {
                std::string fullPath = dir + "/" + name;
                if (accessible(fullPath, true))
                    return fullPath;
            }
            if (end == std::string::npos) break;
            start = end + 1;
        }
#endif
        return "";
    }

    static void writeSyslink(const std::string& syslink, const std::string& target) {
#ifdef _WIN32
        FILE* f = fopen(syslink.c_str(), "w");
        if (f) { fprintf(f, "%s", target.c_str()); fclose(f); }
        else fprintf(stderr, "TAN: could not cache terminal path to %s\n", syslink.c_str());
#else
        unlink(syslink.c_str());
        if (symlink(target.c_str(), syslink.c_str()) != 0)
            fprintf(stderr, "TAN: could not cache terminal path to %s: %s\n",
                    syslink.c_str(), strerror(errno));
#endif
    }

#if defined(__APPLE__)
    static std::string escapeAppleScript(const std::string& s) {
        std::string out;
        for (size_t i = 0; i < s.size(); ++i) {
            char c = s[i];
            if (c == '\\' || c == '"') out += '\\';
            out += c;
        }
        return out;
    }
#endif

    static void init() {
        if (initialized) return;

#ifndef _WIN32
        signal(SIGCHLD, SIG_IGN);
#endif

#if defined(__APPLE__)
        if (!accessible("/usr/bin/osascript", true)) {
            fprintf(stderr, "TAN: /usr/bin/osascript not found, cannot open Terminal\n");
            terminalPath = "";
        } else {
            terminalPath = "/usr/bin/osascript";
        }
        initialized = true;
        return;
#else
        const char* envTerm = getenv("TERMINAL");
        if (envTerm && *envTerm && accessible(envTerm, true)) {
            terminalPath = envTerm;
            writeSyslink(getSyslinkPath(), terminalPath);
            initialized = true;
            return;
        }

        std::string syslink = getSyslinkPath();

        if (accessible(syslink, false)) {
#ifdef _WIN32
            FILE* f = fopen(syslink.c_str(), "r");
            if (f) {
                char buffer[PATH_MAX];
                if (fgets(buffer, sizeof(buffer), f)) {
                    buffer[strcspn(buffer, "\n")] = 0;
                    terminalPath = std::string(buffer);
                }
                fclose(f);
            }
#else
            char buffer[PATH_MAX];
            ssize_t len = readlink(syslink.c_str(), buffer, sizeof(buffer) - 1);
            if (len != -1) {
                buffer[len] = '\0';
                terminalPath = std::string(buffer);
            }
#endif
            if (!terminalPath.empty() && accessible(terminalPath, true)) {
                initialized = true;
                return;
            }
        }

#ifdef _WIN32
        const char* absolutePaths[] = {
            "C:\\Windows\\System32\\wt.exe",
            "C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe",
            "C:\\Windows\\System32\\cmd.exe",
            NULL
        };
#else
        const char* absolutePaths[] = {
            "/usr/bin/konsole",
            "/usr/bin/gnome-terminal",
            "/usr/bin/xfce4-terminal",
            "/usr/bin/qterminal",
            "/usr/bin/mate-terminal",
            "/usr/bin/terminator",
            "/usr/bin/alacritty",
            "/usr/bin/xterm",
            "/usr/bin/x-terminal-emulator",
            NULL
        };
#endif
        for (int i = 0; absolutePaths[i]; i++) {
            if (accessible(absolutePaths[i], true)) {
                terminalPath = absolutePaths[i];
                writeSyslink(syslink, terminalPath);
                initialized = true;
                return;
            }
        }

#ifdef _WIN32
        const char* names[] = { "wt.exe", "powershell.exe", "cmd.exe", NULL };
#else
        const char* names[] = {
            "konsole", "gnome-terminal", "xfce4-terminal",
            "qterminal", "mate-terminal", "terminator",
            "alacritty", "xterm", NULL
        };
#endif
        for (int i = 0; names[i]; i++) {
            std::string path = findInPath(names[i]);
            if (!path.empty()) {
                terminalPath = path;
                writeSyslink(syslink, terminalPath);
                initialized = true;
                return;
            }
        }

        fprintf(stderr, "TAN: no terminal found, symlink one to: %s\n", syslink.c_str());
        terminalPath = "";
        initialized = true;
#endif
    }

#ifndef _WIN32
    static pid_t spawnRaw(const std::vector<std::string>& args) {
        if (args.empty()) return -1;

        std::vector<char*> argv;
        argv.reserve(args.size() + 1);
        for (size_t i = 0; i < args.size(); ++i)
            argv.push_back(const_cast<char*>(args[i].c_str()));
        argv.push_back(NULL);

        posix_spawn_file_actions_t actions;
        posix_spawn_file_actions_init(&actions);
        int devnull = open("/dev/null", O_RDWR);
        if (devnull >= 0)
            posix_spawn_file_actions_adddup2(&actions, devnull, STDIN_FILENO);

        pid_t pid;
        int rc = posix_spawn(&pid, argv[0], &actions, NULL, argv.data(), environ);

        if (devnull >= 0) close(devnull);
        posix_spawn_file_actions_destroy(&actions);

        if (rc != 0) {
            fprintf(stderr, "TAN: failed to launch '%s': %s\n", argv[0], strerror(rc));
            return -1;
        }
        return pid;
    }

    static bool spawnDetached(const std::vector<std::string>& args) {
        return spawnRaw(args) > 0;
    }

    static std::vector<std::string> buildTerminalArgv(const std::string& innerCmd) {
        std::vector<std::string> argv;
#if defined(__APPLE__)
        argv.push_back(terminalPath);
        argv.push_back("-e");
        argv.push_back("tell application \"Terminal\" to do script \"" +
                       escapeAppleScript(innerCmd) + "\"");
#else
        std::string name = terminalPath.substr(terminalPath.find_last_of('/') + 1);
        argv.push_back(terminalPath);
        if (name == "gnome-terminal" || name == "xfce4-terminal") {
            argv.push_back("--");
            argv.push_back("bash");
            argv.push_back("-c");
            argv.push_back(innerCmd);
        } else if (name == "konsole") {
            argv.push_back("-e");
            argv.push_back("bash");
            argv.push_back("-c");
            argv.push_back(innerCmd);
        } else {
            argv.push_back("-e");
            argv.push_back("sh");
            argv.push_back("-c");
            argv.push_back(innerCmd);
        }
#endif
        return argv;
    }
#endif

public:
    static std::string term_bin_path() {
        init();
        return terminalPath;
    }

    static bool tanrun(const std::string& cmd) {
        init();
        if (terminalPath.empty()) {
            fprintf(stderr, "TAN: tanrun failed, no terminal available\n");
            return false;
        }

#ifdef _WIN32
        std::string args = "/c \"" + cmd + " & pause\"";
        HINSTANCE r = ShellExecuteA(NULL, "open", terminalPath.c_str(),
                                    args.c_str(), NULL, SW_SHOWNORMAL);
        if (reinterpret_cast<INT_PTR>(r) <= 32) {
            fprintf(stderr, "TAN: ShellExecuteA failed (error %lu)\n", GetLastError());
            return false;
        }
        return true;
#else
        std::string inner = "echo 'Running: " + cmd + "' && " + cmd +
                            "; echo; read -p 'Press Enter'";
        return spawnDetached(buildTerminalArgv(inner));
#endif
    }

    static bool tanrunsu(const std::string& cmd) {
        init();
        if (terminalPath.empty()) {
            fprintf(stderr, "TAN: tanrunsu failed, no terminal available\n");
            return false;
        }

#ifdef _WIN32
        std::string args = "/c \"" + cmd + " & pause\"";
        SHELLEXECUTEINFOA sei = { sizeof(sei) };
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.lpVerb = "runas";
        sei.lpFile = "cmd.exe";
        std::string params = "/c " + terminalPath + " " + args;
        sei.lpParameters = params.c_str();
        sei.nShow = SW_SHOWNORMAL;
        if (!ShellExecuteExA(&sei)) {
            fprintf(stderr, "TAN: ShellExecuteExA failed (error %lu)\n", GetLastError());
            return false;
        }
        return true;
#else
        std::string inner = "echo 'Running as root: " + cmd + "' && sudo " + cmd +
                            "; echo; read -p 'Press Enter'";
        return spawnDetached(buildTerminalArgv(inner));
#endif
    }

#if !defined(_WIN32) && !defined(__APPLE__)
    static bool tanrunWait(const std::string& cmd, int* exitCode = NULL) {
        init();
        if (terminalPath.empty()) {
            fprintf(stderr, "TAN: tanrunWait failed, no terminal available\n");
            return false;
        }

        char tmpl[] = "/tmp/tan_exit_XXXXXX";
        int fd = mkstemp(tmpl);
        if (fd < 0) {
            fprintf(stderr, "TAN: mkstemp failed: %s\n", strerror(errno));
            return false;
        }
        close(fd);
        std::string exitFile(tmpl);

        std::string inner = "echo 'Running: " + cmd + "' && " + cmd +
                            "; echo $? > '" + exitFile + "'" +
                            "; echo; read -p 'Press Enter'";

        signal(SIGCHLD, SIG_DFL);
        pid_t pid = spawnRaw(buildTerminalArgv(inner));
        int status = 0;
        bool ok = false;
        if (pid > 0) {
            ok = (waitpid(pid, &status, 0) == pid);
            if (!ok)
                fprintf(stderr, "TAN: waitpid failed: %s\n", strerror(errno));
        }
        signal(SIGCHLD, SIG_IGN);

        if (ok && exitCode) {
            *exitCode = -1;
            FILE* f = fopen(exitFile.c_str(), "r");
            if (f) {
                int code;
                if (fscanf(f, "%d", &code) == 1) *exitCode = code;
                fclose(f);
            } else {
                fprintf(stderr, "TAN: could not read exit code from %s\n", exitFile.c_str());
            }
        }
        unlink(exitFile.c_str());
        return ok;
    }
#endif
};

std::string TAN::terminalPath = "";
bool TAN::initialized = false;
std::string TAN::configDir = "";

#endif
