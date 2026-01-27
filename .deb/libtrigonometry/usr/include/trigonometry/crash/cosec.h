#ifndef COSEC_H
#define COSEC_H

#include "cos.h"
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
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTimer>
#include <iostream>

class COSEC;

class CrashOrgMan {
private:
    COS* logger;
    QMainWindow* mainWindow;
    QIcon windowIcon;
    QString windowTitle;
    bool crashHandlerActive;

    inline CrashOrgMan() : logger(nullptr), mainWindow(nullptr), crashHandlerActive(false) {
        logger = new COS();
        logger->setCrashCallback([this](const CrashInfo& info) { handleCrash(info); });
    }

    inline ~CrashOrgMan() { delete logger; }

    void handleCrash(const CrashInfo& crashInfo);

public:
    inline static CrashOrgMan& instance() {
        static CrashOrgMan inst;
        return inst;
    }

    inline void registerWindow(QMainWindow* win) {
        if (win) {
            mainWindow = win;
            windowIcon = win->windowIcon();
            windowTitle = win->windowTitle();
        }
    }

    inline void updateWindowInfo() {
        if (mainWindow) {
            windowIcon = mainWindow->windowIcon();
            windowTitle = mainWindow->windowTitle();
        }
    }

    inline const QIcon& getIcon() const { return windowIcon; }
    inline const QString& getTitle() const { return windowTitle; }

    CrashOrgMan(const CrashOrgMan&) = delete;
    CrashOrgMan& operator=(const CrashOrgMan&) = delete;
};

#define REG_CRASH() CrashOrgMan::instance().registerWindow(this)

class COSEC : public QDialog {

private:
    CrashInfo crashInfo;
    QString applicationPath;
    QIcon windowIcon;
    QString windowTitle;

    inline void setupUI() {
        setWindowTitle(QString::fromStdString(crashInfo.executableName) + " - Crash Report");
        setMinimumSize(400, 350);
        resize(750, 480);
        setWindowIcon(QIcon::fromTheme("folder-crash-symbolic"));
        setAttribute(Qt::WA_DeleteOnClose);

        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(10, 10, 10, 10);
        mainLayout->setSpacing(5);

        QToolBox* toolBox = new QToolBox();
        toolBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mainLayout->addWidget(toolBox);

        toolBox->addItem(createCrashReporterPage(), QIcon::fromTheme("dialog-warning"), "Crash Report");
        toolBox->addItem(createDetailsPage(), QIcon::fromTheme("dialog-information"), "Details");
        toolBox->addItem(createLogsPage(), QIcon::fromTheme("text-x-generic"), "Logs");

        toolBox->setCurrentIndex(0);
    }

    inline QWidget* createLogsPage() {
        QWidget* page = new QWidget();
        QVBoxLayout* outerLayout = new QVBoxLayout(page);
        outerLayout->setContentsMargins(15, 15, 15, 15);
        outerLayout->setSpacing(12);

        outerLayout->addWidget(new QLabel("<h3>Full Application Logs</h3>"));

        QHBoxLayout* mainLayout = new QHBoxLayout();
        mainLayout->setSpacing(20);

        QTextEdit* logText = new QTextEdit();
        logText->setReadOnly(true);
        logText->setPlainText(QString::fromStdString(crashInfo.logContent));
        logText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        logText->setFont(QFont("Monospace", 9));

        QWidget* buttonsWidget = new QWidget();
        buttonsWidget->setFixedWidth(120);
        QVBoxLayout* buttonsLayout = new QVBoxLayout(buttonsWidget);
        buttonsLayout->setContentsMargins(0, 0, 0, 0);
        buttonsLayout->setSpacing(10);

        QPushButton* copyBtn = new QPushButton("Copy");
        copyBtn->setIcon(QIcon::fromTheme("edit-copy"));
        copyBtn->setMinimumHeight(20);
        copyBtn->setMinimumWidth(20);
        copyBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        connect(copyBtn, &QPushButton::clicked, [logText]() {
            QApplication::clipboard()->setText(logText->toPlainText());
            QMessageBox::information(nullptr, "Copied", "Logs copied to clipboard.");
        });

        QPushButton* saveBtn = new QPushButton("Save");
        saveBtn->setIcon(QIcon::fromTheme("document-save"));
        saveBtn->setMinimumHeight(20);
        saveBtn->setMinimumWidth(20);
        saveBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        connect(saveBtn, &QPushButton::clicked, [this]() {
            QString ts = QString::fromStdString(crashInfo.timestamp)
            .replace("/", "").replace(" ", "_").replace(":", "");

            QString defName = QString::fromStdString(crashInfo.executableName) + "_crash_" + ts + ".log";
            QString fname = QFileDialog::getSaveFileName(this, "Save Crash Log As",
                                                         QDir::homePath() + "/" + defName, "Log Files (*.log);;All Files (*)");

            if (!fname.isEmpty()) {
                QFile::copy(QString::fromStdString(crashInfo.logPath), fname) ?
                    QMessageBox::information(this, "Success", "Log file saved successfully.") :
                    QMessageBox::warning(this, "Error", "Failed to save log file.");
            }
        });

        QPushButton* openBtn = new QPushButton("Folder");
        openBtn->setIcon(QIcon::fromTheme("folder-open"));
        openBtn->setMinimumHeight(20);
        openBtn->setMinimumWidth(20);

        openBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        connect(openBtn, &QPushButton::clicked, [this]() {
            QFileInfo fi(QString::fromStdString(crashInfo.logPath));
            if (!QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absolutePath()))) {
                QMessageBox::warning(this, "Error", "Failed to open log folder.");
            }
        });

        buttonsLayout->addWidget(copyBtn, 1);
        buttonsLayout->addWidget(saveBtn, 1);
        buttonsLayout->addWidget(openBtn, 1);

        mainLayout->addWidget(buttonsWidget);
        mainLayout->addWidget(logText, 1);
        outerLayout->addLayout(mainLayout, 1);

        return page;
    }
    inline QWidget* createDetailsPage() {
        QWidget* page = new QWidget();
        QHBoxLayout* mainLayout = new QHBoxLayout(page);
        mainLayout->setContentsMargins(15, 15, 15, 15);
        mainLayout->setSpacing(20);

        QWidget* leftWidget = new QWidget();
        QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
        leftLayout->setContentsMargins(0, 0, 0, 0);
        leftLayout->setSpacing(15);
        leftLayout->setAlignment(Qt::AlignTop);

        QLabel* iconLabel = new QLabel();
        iconLabel->setPixmap(windowIcon.isNull() ?
                                 QIcon::fromTheme("dialog-error").pixmap(128, 128) :
                                 windowIcon.pixmap(128, 128));
        iconLabel->setFixedSize(180, 180);
        leftLayout->addWidget(iconLabel, 0, Qt::AlignLeft);

        leftLayout->addWidget(new QLabel("<h3>This application has been crashed</h3>"));

        QLabel* sigLabel = new QLabel("<b>Signal:</b> " + QString::fromStdString(crashInfo.signalName));
        sigLabel->setWordWrap(true);
        leftLayout->addWidget(sigLabel);

        QLabel* binLabel = new QLabel("<b>Binary:</b> " + QString::fromStdString(crashInfo.executableName));
        binLabel->setWordWrap(true);
        leftLayout->addWidget(binLabel);

        if (!windowTitle.isEmpty()) {
            QLabel* titleLabel = new QLabel("<b>Window Title:</b> " + windowTitle);
            titleLabel->setWordWrap(true);
            leftLayout->addWidget(titleLabel);
        }

        leftLayout->addStretch();
        mainLayout->addWidget(leftWidget, 1);

        QWidget* rightWidget = new QWidget();
        QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
        rightLayout->setContentsMargins(0, 0, 0, 0);
        rightLayout->setSpacing(15);
        rightLayout->setAlignment(Qt::AlignTop);

        rightLayout->addWidget(new QLabel("<h4>Session Informations</h4>"));
        rightLayout->addWidget(new QLabel("<hr>"));

        auto addDetail = [&](const char* label, const QString& value) {
            QLabel* lbl = new QLabel(QString("<b>%1:</b> %2").arg(label).arg(value));
            lbl->setWordWrap(true);
            lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
            rightLayout->addWidget(lbl);
        };

        addDetail("Started", QString::fromStdString(crashInfo.startTime));
        addDetail("Crashed", QString::fromStdString(crashInfo.timestamp));
        addDetail("Log File", QString::fromStdString(crashInfo.logPath));

        rightLayout->addSpacing(20);

        QLabel* sessLabel = new QLabel("<b>Session Duration:</b>");
        rightLayout->addWidget(sessLabel);

        long long ms = crashInfo.sessionDurationMs;
        QString timerDisplay = QString("%1:%2:%3.%4")
                                   .arg(ms / 3600000, 2, 10, QChar('0'))
                                   .arg((ms / 60000) % 60, 2, 10, QChar('0'))
                                   .arg((ms / 1000) % 60, 2, 10, QChar('0'))
                                   .arg((ms / 10) % 100, 2, 10, QChar('0'));

        QLCDNumber* lcd = new QLCDNumber();
        lcd->setDigitCount(12);
        lcd->setSegmentStyle(QLCDNumber::Flat);
        lcd->display(timerDisplay);
        lcd->setMinimumHeight(70);
        lcd->setMaximumHeight(100);
        rightLayout->addWidget(lcd);

        QLabel* formatLabel = new QLabel("(HH:MM:SS.CS)");
        formatLabel->setStyleSheet("color: gray; font-size: 10px;");
        rightLayout->addWidget(formatLabel);

        rightLayout->addStretch();
        mainLayout->addWidget(rightWidget);

        return page;
    }
    inline QWidget* createCrashReporterPage() {
        QWidget* page = new QWidget();
        QVBoxLayout* mainLayout = new QVBoxLayout(page);
        mainLayout->setContentsMargins(15, 15, 15, 15);
        mainLayout->setSpacing(12);

        QString titleText = windowTitle.isEmpty() ?
                                QString::fromStdString(crashInfo.executableName) + " has crashed" :
                                windowTitle + " has crashed";

        QLabel* title = new QLabel("<h2>" + titleText + "</h2>");
        title->setWordWrap(true);
        mainLayout->addWidget(title);

        QLabel* desc = new QLabel(
            "The application encountered a fatal error and needs to close. "
            "Below is the stack trace that may help identify the issue.");
        desc->setWordWrap(true);
        desc->setStyleSheet("color: #555;");
        mainLayout->addWidget(desc);

        mainLayout->addSpacing(10);
        mainLayout->addWidget(new QLabel("<b>Stack Trace:</b>"));

        QHBoxLayout* loggerButtonsLayout = new QHBoxLayout();
        loggerButtonsLayout->setSpacing(20);

        QTextEdit* stackText = new QTextEdit();
        stackText->setReadOnly(true);
        stackText->setFont(QFont("Monospace", 9));
        if (crashInfo.stackTrace.empty()) {
            stackText->setPlainText("No stack trace available (Windows or unavailable backtrace)");
            stackText->setStyleSheet("background-color: #f5f5f5; color: #888;");
        } else {
            stackText->setPlainText(QString::fromStdString(crashInfo.stackTrace));
        }
        stackText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        loggerButtonsLayout->addWidget(stackText, 1);

        QWidget* buttonsWidget = new QWidget();
        buttonsWidget->setFixedWidth(120);
        QVBoxLayout* buttonsLayout = new QVBoxLayout(buttonsWidget);
        buttonsLayout->setContentsMargins(0, 0, 0, 0);
        buttonsLayout->setSpacing(9);
        QPushButton* restartBtn = new QPushButton("Restart");
        restartBtn->setIcon(QIcon::fromTheme("system-reboot"));
        restartBtn->setMinimumHeight(81);
        restartBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        restartBtn->setStyleSheet(
            "QPushButton {"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "    border: 2px solid #00C70A;"
            "    border-radius: 4px;"

            "}"
            );

        QPushButton* closeBtn = new QPushButton("Close");
        closeBtn->setIcon(QIcon::fromTheme("process-stop"));
        closeBtn->setMinimumHeight(81);
        closeBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        closeBtn->setStyleSheet(
            "QPushButton {"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "    border: 2px solid #C70000;"
            "    border-radius: 4px;"

            "}"
            );

        connect(restartBtn, &QPushButton::clicked, [this]() {
            COS::Tri_reset();
        });

        connect(closeBtn, &QPushButton::clicked, [this]() {
            accept();
            COS::Tri_term();
        });

        buttonsLayout->addWidget(restartBtn, 1);
        buttonsLayout->addWidget(closeBtn, 1);

        loggerButtonsLayout->addWidget(buttonsWidget);
        mainLayout->addLayout(loggerButtonsLayout, 1);

        return page;
    }

public:
    explicit COSEC(const CrashInfo& info, const QString& path, const QIcon& icon, const QString& title)
        : QDialog(nullptr), crashInfo(info), applicationPath(path), windowIcon(icon), windowTitle(title) {
        setupUI();
    }
};
inline void CrashOrgMan::handleCrash(const CrashInfo& crashInfo) {
    if (crashHandlerActive) {
        std::cerr << "Recursive crash detected, terminated." << std::endl;
        std::exit(crashInfo.signalNumber);
    }
    crashHandlerActive = true;

    std::cout << "\nCrash handler called.\nSignal: " << crashInfo.signalName
              << "\nTime: " << crashInfo.timestamp << std::endl;

    updateWindowInfo();

    if (mainWindow) {
        mainWindow->hide();
        mainWindow->deleteLater();
        mainWindow = nullptr;
    }

    COSEC* dialog = new COSEC(crashInfo, QCoreApplication::applicationFilePath(), windowIcon, windowTitle);

    QObject::connect(dialog, &QDialog::finished, [](int result) {
        std::cout << "\nCrash dialog closed: " << result << std::endl;
        QApplication::quit();
        std::exit(0);
    });

    dialog->show();
    dialog->exec();
    std::exit(0);
}


#endif // COSEC_H
