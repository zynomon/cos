#include "cosec.h"

class TestCrashGUI : public QMainWindow {
    Q_OBJECT

public:
    TestCrashGUI() {

        COSEC_REGISTER_WINDOW();

        setWindowTitle("Crash Test Application");
        setMinimumSize(450, 350);
        resize(500, 400);
        setWindowIcon(QIcon::fromTheme("application-x-deb"));

        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QVBoxLayout* layout = new QVBoxLayout(centralWidget);
        layout->setSpacing(15);
        layout->setContentsMargins(20, 20, 20, 20);

        QLabel* titleLabel = new QLabel("Crash Test Application");
        titleLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(titleLabel);

        QLabel* infoLabel = new QLabel(
            "COS is the logger for all things"
            " while COSEC is the gui crash handler"
            );
        infoLabel->setWordWrap(true);
        infoLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(infoLabel);

        layout->addSpacing(20);

        QPushButton* segfaultBtn = new QPushButton("Test Segmentation Fault (SIGSEGV)");
        segfaultBtn->setMinimumHeight(50);
        connect(segfaultBtn, &QPushButton::clicked, this, &TestCrashGUI::testSegfault);
        layout->addWidget(segfaultBtn);

        QPushButton* divideBtn = new QPushButton("Test Division by Zero (SIGFPE)");
        divideBtn->setMinimumHeight(50);
        connect(divideBtn, &QPushButton::clicked, this, &TestCrashGUI::testDivisionByZero);
        layout->addWidget(divideBtn);

        QPushButton* abortBtn = new QPushButton("Test Abort (SIGABRT)");
        abortBtn->setMinimumHeight(50);
        connect(abortBtn, &QPushButton::clicked, this, &TestCrashGUI::testAbort);
        layout->addWidget(abortBtn);

        layout->addStretch();

        QPushButton* logBtn = new QPushButton("Write Test Log");
        logBtn->setMinimumHeight(40);
        connect(logBtn, &QPushButton::clicked, this, &TestCrashGUI::writeTestLog);
        layout->addWidget(logBtn);
    }

private slots:
    void testSegfault() {
        std::cout << "User triggered segmentation fault test..." << std::endl;
        std::cout << "This will cause a SIGSEGV signal." << std::endl;
        int* ptr = nullptr;
        *ptr = 42;
    }

    void testDivisionByZero() {
        std::cout << "User triggered division by zero test..." << std::endl;
        std::cout << "This will cause a SIGFPE signal." << std::endl;
        int x = 5;
        int y = 0;
        volatile int z = x / y;
        (void)z;
    }

    void testAbort() {
        std::cout << "User triggered abort test..." << std::endl;
        std::cout << "This will cause a SIGABRT signal." << std::endl;
        std::abort();
    }

    void writeTestLog() {
        std::cout << "Test log entry" << std::endl;
        std::cout << "This is a normal log message." << std::endl;
    }
};

#include "test.moc"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("CrashTestApp");
    app.setOrganizationName("ProductionLogger");

    TestCrashGUI mainWindow;
    mainWindow.show();

    return app.exec();
}
