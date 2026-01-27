#ifndef COS_H
#define COS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <csignal>
#include <chrono>
#include <iomanip>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#include <execinfo.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#endif
inline const std::string& irs() {
    static const std::string irs = "\n\n▒▒▒█   ▒▒▒█   ▒▒▒█   █▒▒█   █▒▒▒   █▒▒▒   █▒▒▒   █▒▒▒\n\n";
    return irs;
}

struct CrashInfo {
    std::string signalName;
    int signalNumber;
    std::string stackTrace;
    std::string timestamp;
    std::string logPath;
    std::string logContent;
    std::string executableName;
    std::string startTime;
    long long sessionDurationMs;

    std::string getFormattedDuration() const {
        long long hours = sessionDurationMs / (1000 * 60 * 60);
        long long minutes = (sessionDurationMs / (1000 * 60)) % 60;
        long long seconds = (sessionDurationMs / 1000) % 60;
        long long centiseconds = (sessionDurationMs / 10) % 100;

        char buffer[12];
        snprintf(buffer, sizeof(buffer), "%02lld:%02lld:%02lld:%02lld",
                 hours, minutes, seconds, centiseconds);
        return std::string(buffer);
    }
};

class COS {
public:
    using CrashCallback = std::function<void(const CrashInfo&)>;

private:
    std::stringstream capturedOutput;
    std::streambuf* originalCoutBuffer;
    std::streambuf* originalCerrBuffer;
    std::string logPath;
    bool logSaved;
    std::string executableName;
    std::string startTime;
    std::string stackTrace;
    CrashCallback crashCallback;

    std::chrono::system_clock::time_point startTimePoint;

    inline static COS* globalInstance = nullptr;

    class TeeStreambuf : public std::streambuf {
    private:
        std::streambuf* console;
        std::streambuf* captureBuffer;

    public:
        TeeStreambuf(std::streambuf* console, std::streambuf* captureBuffer)
            : console(console), captureBuffer(captureBuffer) {}

    protected:
        inline int overflow(int c) override {
            if (c == EOF) return !EOF;
            console->sputc(c);
            captureBuffer->sputc(c);
            return c;
        }

        inline std::streamsize xsputn(const char* s, std::streamsize n) override {
            console->sputn(s, n);
            captureBuffer->sputn(s, n);
            console->pubsync();
            return n;
        }
    };

    TeeStreambuf* coutBuffer;
    TeeStreambuf* cerrBuffer;

    inline std::string getTimestampForFilename() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tm;

#ifdef _WIN32
        localtime_s(&tm, &time);
#else
        localtime_r(&time, &tm);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
        return oss.str();
    }

    inline std::string getTimestampForLog() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tm;

#ifdef _WIN32
        localtime_s(&tm, &time);
#else
        localtime_r(&time, &tm);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y/%m/%d %H:%M:%S");
        return oss.str();
    }

    std::string getExecutableNameInternal() const {
#ifdef _WIN32
        char path[MAX_PATH];
        GetModuleFileNameA(NULL, path, MAX_PATH);
        std::string fullPath(path);
        size_t pos = fullPath.find_last_of("\\/");
        std::string exeName = (pos == std::string::npos) ? fullPath : fullPath.substr(pos + 1);
        size_t extPos = exeName.find_last_of('.');
        if (extPos != std::string::npos) {
            exeName = exeName.substr(0, extPos);
        }
        return exeName;
#else
        char path[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
        if (count != -1) {
            path[count] = '\0';
            std::string fullPath(path);
            size_t pos = fullPath.find_last_of('/');
            return (pos == std::string::npos) ? fullPath : fullPath.substr(pos + 1);
        }
        return "app";
#endif
    }

    inline std::string getTempDir() const {
        std::string appid = executableName;
        std::string timestamp = getTimestampForFilename();

#ifdef _WIN32
        const char* tempDir = std::getenv("TEMP");
        if (!tempDir) tempDir = std::getenv("TMP");
        if (!tempDir) tempDir = "C:\\Temp";
        return std::string(tempDir) + "\\" + appid + "_" + timestamp + ".log";
#else
        return "/tmp/" + appid + "_" + timestamp + ".log";
#endif
    }

#ifndef _WIN32
    std::string captureStackTrace() const {
        const int MAX_FRAMES = 64;

        void* buffer[MAX_FRAMES];
        int numFrames = backtrace(buffer, MAX_FRAMES);

        std::stringstream ss;
        char** symbols = backtrace_symbols(buffer, numFrames);

        if (symbols) {
            for (int i = 0; i < numFrames; i++) {
                ss << symbols[i] << "\n";
            }
            free(symbols);
        }
        return ss.str();
    }
#endif

    void setupSignalHandlers() {
        std::signal(SIGTERM, signalHandler);
        std::signal(SIGINT, signalHandler);
        std::signal(SIGABRT, signalHandler);
        std::signal(SIGFPE, signalHandler);
        std::signal(SIGILL, signalHandler);
        std::signal(SIGSEGV, signalHandler);

#ifndef _WIN32
        std::signal(SIGBUS, signalHandler);
        std::signal(SIGQUIT, signalHandler);
        std::signal(SIGTRAP, signalHandler);
#endif
    }

    static void signalHandler(int sigNum) {
        if (globalInstance) {
            globalInstance->handleSignal(sigNum);
        }
    }

    inline std::string getSignalName(int sigNum) const {
        switch(sigNum) {
        case SIGTERM: return "SIGTERM";
        case SIGINT: return "SIGINT";
        case SIGABRT: return "SIGABRT";
        case SIGFPE: return "SIGFPE";
        case SIGILL: return "SIGILL";
        case SIGSEGV: return "SIGSEGV";
#ifndef _WIN32
        case SIGBUS: return "SIGBUS";
        case SIGQUIT: return "SIGQUIT";
        case SIGTRAP: return "SIGTRAP";
#endif
        default: return "Signal " + std::to_string(sigNum);
        }
    }

    void handleSignal(int sigNum) {
        std::string signalName = getSignalName(sigNum);
        std::string currentTime = getTimestampForLog();

        std::cout << "\n!!! A " << signalName << " SIGNAL FAILURE CAUGHT !!!" << std::endl;

#ifndef _WIN32
        stackTrace = captureStackTrace();
        if (!stackTrace.empty()) {
            std::cout << "\n The Crash Signal  Trace; " << irs() << stackTrace << irs() ;
        }
#endif

        saveLog("Crashed: " + signalName);

        if (crashCallback) {

            auto endTimePoint = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                endTimePoint - startTimePoint
                );

            CrashInfo info;
            info.signalName = signalName;
            info.signalNumber = sigNum;
            info.stackTrace = stackTrace;
            info.timestamp = currentTime;
            info.logPath = logPath;
            info.logContent = capturedOutput.str();
            info.executableName = executableName;
            info.startTime = startTime;
            info.sessionDurationMs = duration.count();

            crashCallback(info);
        } else {
            std::exit(sigNum);
        }
    }

public:
    COS() : logSaved(false), crashCallback(nullptr), coutBuffer(nullptr), cerrBuffer(nullptr) {
        executableName = getExecutableNameInternal();
        logPath = getTempDir();

        startTimePoint = std::chrono::system_clock::now();
        startTime = getTimestampForLog();

        originalCoutBuffer = std::cout.rdbuf();
        coutBuffer = new TeeStreambuf(originalCoutBuffer, capturedOutput.rdbuf());
        std::cout.rdbuf(coutBuffer);

        originalCerrBuffer = std::cerr.rdbuf();
        cerrBuffer = new TeeStreambuf(originalCerrBuffer, capturedOutput.rdbuf());
        std::cerr.rdbuf(cerrBuffer);

        globalInstance = this;
        setupSignalHandlers();

        std::cout << "COS: " << logPath << std::endl;
    }

    ~COS() {
        if (!logSaved) {
            saveLog("Normal exit");
        }

        std::cout.rdbuf(originalCoutBuffer);
        std::cerr.rdbuf(originalCerrBuffer);

        delete coutBuffer;
        delete cerrBuffer;

        if (globalInstance == this) {
            globalInstance = nullptr;
        }
    }

    inline void setCrashCallback(CrashCallback callback) {
        crashCallback = callback;
    }

    void saveLog(const std::string& exitReason) {
        if (logSaved) return;
        logSaved = true;

        std::cout.rdbuf(originalCoutBuffer);
        std::cerr.rdbuf(originalCerrBuffer);

        auto endTimePoint = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTimePoint - startTimePoint
            );
        long long durationMs = duration.count();

        long long hours = durationMs / (1000 * 60 * 60);
        long long minutes = (durationMs / (1000 * 60)) % 60;
        long long seconds = (durationMs / 1000) % 60;
        long long centiseconds = (durationMs / 10) % 100;

        char durationBuffer[32];
        snprintf(durationBuffer, sizeof(durationBuffer), "%02lld:%02lld:%02lld:%02lld",
                 hours, minutes, seconds, centiseconds);

        std::ofstream logFile(logPath);
        if (logFile.is_open()) {
            logFile << "--------------------------------------------- DATA ----------------------------------------------\n"
                    << "App: " << executableName << "\n"
                    << "Start: " << startTime << "\n"
                    << "Exit: " << exitReason << " at " << getTimestampForLog() << "\n"
                    << "Duration: " << durationBuffer << " (HH:MM:SS:CS)\n\n"
                    << "----------------------------------------- CAPTURED LOGS -----------------------------------------\n"
                    << capturedOutput.str();

            if (!stackTrace.empty()) {
                logFile  <<" THE SIGNAL FAULT STACK TRACE :" << irs() << stackTrace << irs();
            }

            logFile.close();
        }
    }

    inline const std::string& getExecutableName() const { return executableName; }
    inline const std::string& getLogPath() const { return logPath; }
    inline const std::string& getStartTime() const { return startTime; }
    inline const std::string& getStackTrace() const { return stackTrace; }
    inline std::string getLogContent() const { return capturedOutput.str(); }

    static void Tri_reset() {
        if (globalInstance) {
            globalInstance->saveLog("Application restart initiated");
        }

#ifdef _WIN32

        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);

        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi;

        if (CreateProcessA(
                exePath,

                NULL,

                NULL,

                NULL,

                FALSE,

                DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP,

                NULL,

                NULL,

                &si,

                &pi

                )) {

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            Sleep(100);
        } else {
            if (globalInstance) {
                globalInstance->saveLog("Failed to restart application");
            }
        }

#else

        pid_t pid = fork();

        if (pid == 0) {

            setsid();

            pid_t pid2 = fork();
            if (pid2 > 0) {

                _exit(0);
            } else if (pid2 == 0) {

                chdir("/");

                close(STDIN_FILENO);
                close(STDOUT_FILENO);
                close(STDERR_FILENO);

                open("/dev/null", O_RDONLY);

                open("/dev/null", O_WRONLY);

                open("/dev/null", O_WRONLY);

                for (int fd = 3; fd < 1024; fd++) {
                    close(fd);
                }

                execl("/proc/self/exe", "/proc/self/exe", (char*)NULL);

                _exit(127);
            } else {

                _exit(1);
            }
        } else if (pid > 0) {

            waitpid(pid, NULL, 0);

            usleep(100000);

        } else {

            if (globalInstance) {
                globalInstance->saveLog("Failed to fork for restart");
            }
        }
#endif

        Tri_term();
        exit(0);

    }

    static void Tri_term() {

        signal(SIGTERM, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        signal(SIGABRT, SIG_DFL);
        signal(SIGFPE, SIG_DFL);
        signal(SIGILL, SIG_DFL);
        signal(SIGSEGV, SIG_DFL);
#ifndef _WIN32
        signal(SIGBUS, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTRAP, SIG_DFL);
#endif

        if (globalInstance) {
            delete globalInstance;
            globalInstance = nullptr;
        }

#ifdef _WIN32
        ExitProcess(0);
#else
        _exit(0);
#endif
    }

    COS(const COS&) = delete;
    COS& operator=(const COS&) = delete;
};


#endif // COS_H
