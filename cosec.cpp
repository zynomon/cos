#include "cos.h"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QToolBox>
#include <QLCDNumber>
#include <QDialog>
#include <QIcon>
#include <QTextEdit>
#include <QFileDialog>
#include <QDesktopServices>
#include <QClipboard>
#include <QFile>
#include <QScrollBar>
#include <QDir>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>
#include <iostream>

COS* g_logger = nullptr;

QMainWindow* g_mainWindow = nullptr;

static bool g_crashHandlerActive = false;

class COSEC : public QDialog {
    Q_OBJECT

private:
    CrashInfo crashInfo;
    QString applicationPath;

    void setupUI() {
        setWindowTitle(QString::fromStdString(crashInfo.executableName) + " has crashed");
        setMinimumSize(600, 400);
        resize(800, 500);
        setAttribute(Qt::WA_DeleteOnClose);

        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(10, 10, 10, 10);
        mainLayout->setSpacing(5);

        QToolBox* toolBox = new QToolBox();
        toolBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mainLayout->addWidget(toolBox);

        toolBox->addItem(createLogsPage(), QIcon::fromTheme("format-justify-left"), "Logs");
        toolBox->addItem(createDetailsPage(), QIcon::fromTheme("mail-read"), "Details");
        toolBox->addItem(createCrashReporterPage(), QIcon::fromTheme("application-exit"), "Crash Reporter");

        toolBox->setCurrentIndex(2);
    }

    QWidget* createLogsPage() {
        QWidget* page = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(page);
        layout->setContentsMargins(10, 10, 10, 10);
        layout->setSpacing(10);

        QTextEdit* logText = new QTextEdit();
        logText->setReadOnly(true);
        logText->setPlainText(QString::fromStdString(crashInfo.logContent));
        logText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        logText->setMinimumHeight(150);
        layout->addWidget(logText, 1);

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->setSpacing(10);

        QPushButton* copyBtn = new QPushButton("Copy");
        copyBtn->setIcon(QIcon::fromTheme("edit-copy"));
        copyBtn->setMinimumWidth(100);
        connect(copyBtn, &QPushButton::clicked, [logText]() {
            QApplication::clipboard()->setText(logText->toPlainText());
        });

        QPushButton* saveBtn = new QPushButton("Save As...");
        saveBtn->setIcon(QIcon::fromTheme("document-save"));
        saveBtn->setMinimumWidth(120);
        connect(saveBtn, &QPushButton::clicked, this, &COSEC::saveLogAs);

        QPushButton* openFolderBtn = new QPushButton("Open Folder");
        openFolderBtn->setIcon(QIcon::fromTheme("folder-open"));
        openFolderBtn->setMinimumWidth(120);
        connect(openFolderBtn, &QPushButton::clicked, this, &COSEC::openLogFolder);

        buttonLayout->addWidget(copyBtn);
        buttonLayout->addWidget(saveBtn);
        buttonLayout->addWidget(openFolderBtn);
        buttonLayout->addStretch();

        layout->addLayout(buttonLayout);
        return page;
    }

    QWidget* createDetailsPage() {
        QWidget* page = new QWidget();
        QVBoxLayout* mainLayout = new QVBoxLayout(page);
        mainLayout->setContentsMargins(10, 10, 10, 10);
        mainLayout->setSpacing(15);

        QHBoxLayout* topLayout = new QHBoxLayout();
        topLayout->setSpacing(15);

        QLabel* iconLabel = new QLabel();
        QIcon appIcon = QApplication::windowIcon();  // not working properly
        if (!appIcon.isNull()) {
            iconLabel->setPixmap(appIcon.pixmap(128, 128));
        } else {
            iconLabel->setPixmap(QIcon::fromTheme("application-x-desktop").pixmap(128, 128));
        }
        iconLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        topLayout->addWidget(iconLabel);

        QWidget* crashInfoWidget = new QWidget();
        QVBoxLayout* crashInfoLayout = new QVBoxLayout(crashInfoWidget);
        crashInfoLayout->setSpacing(5);

        QLabel* signalLabel = new QLabel("Crash Cause: " + QString::fromStdString(crashInfo.signalName));
        signalLabel->setWordWrap(true);
        crashInfoLayout->addWidget(signalLabel);

        QLabel* appNameLabel = new QLabel("Binary name: " + QString::fromStdString(crashInfo.executableName));
        appNameLabel->setWordWrap(true);
        crashInfoLayout->addWidget(appNameLabel);

        topLayout->addWidget(crashInfoWidget, 1);

        mainLayout->addLayout(topLayout);

        QWidget* detailsWidget = new QWidget();
        QVBoxLayout* detailsLayout = new QVBoxLayout(detailsWidget);
        detailsLayout->setSpacing(8);

        auto addDetail = [&](const QString& label, const QString& value) {
            QLabel* detailLabel = new QLabel("<b>" + label + ":</b> " + value);
            detailLabel->setWordWrap(true);
            detailLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
            detailsLayout->addWidget(detailLabel);
        };

        addDetail("Application", QString::fromStdString(crashInfo.executableName));
        addDetail("Started", QString::fromStdString(crashInfo.startTime));
        addDetail("Crashed", QString::fromStdString(crashInfo.timestamp));
        addDetail("Log File", QString::fromStdString(crashInfo.logPath));

        mainLayout->addWidget(detailsWidget);
        mainLayout->addSpacing(20);

        QWidget* lcdWidget = new QWidget();
        QVBoxLayout* lcdLayout = new QVBoxLayout(lcdWidget);
        lcdLayout->setAlignment(Qt::AlignCenter);

        QLabel* sessionLabel = new QLabel("This session lasted about:");
        sessionLabel->setAlignment(Qt::AlignCenter);
        lcdLayout->addWidget(sessionLabel);

        long long totalMs = crashInfo.sessionDurationMs;
        long long hours = totalMs / (1000 * 60 * 60);
        long long minutes = (totalMs / (1000 * 60)) % 60;
        long long seconds = (totalMs / 1000) % 60;
        long long centiseconds = (totalMs / 10) % 100;

        QString timerDisplay = QString("%1:%2:%3:%4")
                                   .arg(hours, 2, 10, QChar('0'))
                                   .arg(minutes, 2, 10, QChar('0'))
                                   .arg(seconds, 2, 10, QChar('0'))
                                   .arg(centiseconds, 2, 10, QChar('0'));

        QLCDNumber* lcdNumber = new QLCDNumber();
        lcdNumber->setDigitCount(11);

        lcdNumber->setSegmentStyle(QLCDNumber::Flat);
        lcdNumber->display(timerDisplay);
        lcdNumber->setMinimumHeight(60);
        lcdNumber->setMaximumHeight(100);
        lcdLayout->addWidget(lcdNumber);

        mainLayout->addWidget(lcdWidget);
        mainLayout->addStretch();

        return page;
    }

    QWidget* createCrashReporterPage() {
        QWidget* page = new QWidget();
        QVBoxLayout* mainLayout = new QVBoxLayout(page);
        mainLayout->setContentsMargins(10, 10, 10, 10);
        mainLayout->setSpacing(10);

        QLabel* title = new QLabel(QString::fromStdString(crashInfo.executableName) + " has crashed");
        title->setWordWrap(true);
        mainLayout->addWidget(title);

        QLabel* stackLabel = new QLabel("Stack Trace:");
        mainLayout->addWidget(stackLabel);

        QTextEdit* stackTraceText = new QTextEdit();
        stackTraceText->setReadOnly(true);
        stackTraceText->setFont(QFont("Monospace", 9));
        if (crashInfo.stackTrace.empty()) {
            stackTraceText->setPlainText("No stack trace available");
        } else {
            stackTraceText->setPlainText(QString::fromStdString(crashInfo.stackTrace));
        }
        stackTraceText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        stackTraceText->setMinimumHeight(150);
        mainLayout->addWidget(stackTraceText, 1);

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->setSpacing(10);

        QPushButton* restartBtn = new QPushButton("Restart Application");
        restartBtn->setIcon(QIcon::fromTheme("system-reboot"));
        restartBtn->setMinimumHeight(40);
        restartBtn->setToolTip("Restart the application");
        connect(restartBtn, &QPushButton::clicked, this, &COSEC::restartApplication);

        QPushButton* closeBtn = new QPushButton("Close Application");
        closeBtn->setIcon(QIcon::fromTheme("process-stop"));
        closeBtn->setMinimumHeight(40);
        closeBtn->setToolTip("Close the application");
        connect(closeBtn, &QPushButton::clicked, this, &COSEC::closeApplication);

        buttonLayout->addWidget(restartBtn);
        buttonLayout->addWidget(closeBtn);

        mainLayout->addLayout(buttonLayout);
        return page;
    }

private slots:
    void saveLogAs() {

        QString timestamp = QString::fromStdString(crashInfo.timestamp);

        timestamp = timestamp.replace("/", "").replace(" ", "_").replace(":", "");

        QString defaultName = QString::fromStdString(crashInfo.executableName) +
                              "_crash_" + timestamp + ".log";

        QString fileName = QFileDialog::getSaveFileName(
            this,
            "Save Crash Log As",
            QDir::homePath() + "/" + defaultName,
            "Log Files (*.log);;All Files (*)"
            );

        if (!fileName.isEmpty()) {
            if (QFile::copy(QString::fromStdString(crashInfo.logPath), fileName)) {
                QMessageBox::information(this, "Success", "Log file saved successfully.");
            } else {
                QMessageBox::warning(this, "Error", "Failed to save log file.");
            }
        }
    }

    void openLogFolder() {
        QFileInfo fileInfo(QString::fromStdString(crashInfo.logPath));
        QString folderPath = fileInfo.absolutePath();

        if (!QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath))) {
            QMessageBox::warning(this, "Error", "Failed to open log folder.");
        }
    }

    void restartApplication() {
        if (!applicationPath.isEmpty()) {
            QProcess::startDetached(applicationPath, QStringList());
            closeApplication();
        } else {
            QMessageBox::warning(this, "Error", "Cannot restart application: path unknown.");
        }
    }

    void closeApplication() {
        accept();

        QTimer::singleShot(100, []() {
            QApplication::quit();
            std::exit(0);
        });
    }

public:
    explicit COSEC(const CrashInfo& info, const QString& appPath, QWidget* parent = nullptr)
        : QDialog(parent), crashInfo(info), applicationPath(appPath) {
        setupUI();
    }
};

class TestCrashGUI : public QMainWindow {  // we are now in test phrase
    Q_OBJECT

public:
    TestCrashGUI() {
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

        g_mainWindow = this;
    }

    ~TestCrashGUI() {
        g_mainWindow = nullptr;
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

void handleApplicationCrash(const CrashInfo& crashInfo) {

    if (g_crashHandlerActive) {
        std::cerr << "Recursive crash detected, terminated." << std::endl;
        std::exit(crashInfo.signalNumber);
    }
    g_crashHandlerActive = true;

    std::cout << "\n Crash handler was called." << std::endl;
    std::cout << "Signal: " << crashInfo.signalName << std::endl;
    std::cout << "Time: " << crashInfo.timestamp << std::endl;
    std::cout << "Log: " << crashInfo.logPath << std::endl;

    if (g_mainWindow) {
        g_mainWindow->hide();
        g_mainWindow->deleteLater();
        g_mainWindow = nullptr;
    }

    QString appPath = QCoreApplication::applicationFilePath();

    COSEC* dialog = new COSEC(crashInfo, appPath);

    QObject::connect(dialog, &QDialog::finished, [](int result) {
        std::cout << "\nCrash dialog closed with result: " << result << std::endl;
        QApplication::quit();
        std::exit(0);
    });

    dialog->show();
    dialog->exec();

    std::exit(0);
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("CrashTestApp");
    app.setOrganizationName("error.os");

    COS logger;
    g_logger = &logger;
    logger.setCrashCallback(handleApplicationCrash);

    TestCrashGUI mainWindow;
    mainWindow.show();

    int result = app.exec();
    return result;
}

#include "cosec.moc"
