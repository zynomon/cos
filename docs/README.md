# Libtrigonometry

---

libtrigonometry is a lightweight linux library, it uses qt and c++ , that tends to contain 6 header ( not much ) 

so what are already created? 
---

## COS <sub>character output streambuffer</sub>  
#### Technology : Raw C++ 
#### OS : Linux, BSD ( havent tested ) , MAC ( havent tested ) , WINDOWS ( havent tested )
#### CMAKE LINKING ID : crash (link in cmake using  Trig::crash )

__***Description:***__  automatically captures all application output and handles fatal signals. It works by intercepting stdout and stderr streams using a custom TeeStreambuf implementation, simultaneously displaying output to the console while recording it to a timestamped log file in the system's temporary directory.
When initialized, COS sets up signal handlers for all major crash signals (SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGBUS, etc.) and begins monitoring the application. On Unix/Linux systems, it captures full stack traces using backtrace() when crashes occur. The logger tracks session duration with centisecond precision and generates comprehensive crash reports.


__***Common uses:***__  COS emits some public signals that are maybe useful,
```cpp
// Restart application cleanly
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
logger.getLogContent();      // Returns all captured output
```


## COSEC <sub>Crash output stream executor</sub>  
#### Technology : Qt6 + C++
#### OS : Linux, BSD ( havent tested ) , MAC ( havent tested ) , WINDOWS ( havent tested )
#### CMAKE LINKING ID : crash (link in cmake using  Trig::crash )

__***Description:***__ captures cos and executes a gui crash reporter
__***Screenshots:**__ This application fetches icon for the window , and more.
<img width="729" height="515" alt="image" src="https://github.com/user-attachments/assets/042bda71-a8e7-481e-a31d-74771b2480ed" />
<img width="766" height="519" alt="image" src="https://github.com/user-attachments/assets/2d54f818-f633-4888-82e0-c5d97fab7cbe" />
<img width="765" height="522" alt="image" src="https://github.com/user-attachments/assets/437b912f-4a0c-42a4-84a0-21db0e34340f" />



### steps;
0. download from here ; coming soon.. wait a day
1. type `find_packages(Trig)`
2. Link yout project against `Trig::crash` ( crash means bundle with cos and cosec )
3. use `#include cosec` in your project
4. in your mainwindowclass ( where your window title defined, add a line `REG_CRASH();` that's it

**more coming soon..***
---
 
