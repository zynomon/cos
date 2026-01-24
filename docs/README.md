# Libtrigonometry

---

libtrigonometry is a lightweight linux library, it uses qt and c++ , that tends to contain 6 header ( not much ) 

so what are already created? 
---

## COS <sub>character output streambuffer</sub>  Technology : Raw C++ 

__***Description:***__  automatically captures all application output and handles fatal signals. It works by intercepting stdout and stderr streams using a custom TeeStreambuf implementation, simultaneously displaying output to the console while recording it to a timestamped log file in the system's temporary directory.
When initialized, COS sets up signal handlers for all major crash signals (SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGBUS, etc.) and begins monitoring the application. On Unix/Linux systems, it captures full stack traces using backtrace() when crashes occur. The logger tracks session duration with centisecond precision and generates comprehensive crash reports.

__***Common uses:**__  COS emits some public signals that are maybe useful ```cpp // Restart application cleanly
COS::Tri_reset();  // Launches new instance and terminates current one

// Terminate application cleanly
COS::Tri_term();   // Resets signal handlers and exits gracefully

// Manual log save (if needed before normal exit)
logger.saveLog("User requested shutdown");

// Access crash information
logger.getExecutableName();  // Returns binary name
logger.getLogPath();         // Returns log file path
logger.getStartTime();       // Returns session start timestamp
logger.getStackTrace();      // Returns captured stack trace (if any)
logger.getLogContent();      // Returns all captured output ```


 
