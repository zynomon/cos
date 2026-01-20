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
#include <QHBoxLayout>
#include <QMessageBox>
#include <QProcess>
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

#define COSEC_REGISTER_WINDOW() CrashOrgMan::instance().registerWindow(this)

class COSEC : public QDialog {
    Q_OBJECT

private:
    CrashInfo crashInfo;
    QString applicationPath;
    QIcon windowIcon;
    QString windowTitle;

    inline void setupUI() {
        setWindowTitle(QString::fromStdString(crashInfo.executableName) + " - Crash Report");
        setMinimumSize(700, 450);
        resize(850, 550);
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
        QVBoxLayout* layout = new QVBoxLayout(page);
        layout->setContentsMargins(15, 15, 15, 15);
        layout->setSpacing(10);

        layout->addWidget(new QLabel("<b>Full Application Logs</b>"));

        QTextEdit* logText = new QTextEdit();
        logText->setReadOnly(true);
        logText->setPlainText(QString::fromStdString(crashInfo.logContent));
        logText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        logText->setMinimumHeight(200);
        logText->setFont(QFont("Monospace", 9));
        layout->addWidget(logText, 1);

        QHBoxLayout* btnLayout = new QHBoxLayout();
        btnLayout->setSpacing(10);

        QPushButton* copyBtn = new QPushButton("Copy to Clipboard");
        copyBtn->setIcon(QIcon::fromTheme("edit-copy"));
        copyBtn->setMinimumWidth(130);
        connect(copyBtn, &QPushButton::clicked, [logText]() {
            QApplication::clipboard()->setText(logText->toPlainText());
            QMessageBox::information(nullptr, "Copied", "Logs copied to clipboard.");
        });

        QPushButton* saveBtn = new QPushButton("Save As...");
        saveBtn->setIcon(QIcon::fromTheme("document-save"));
        saveBtn->setMinimumWidth(120);
        connect(saveBtn, &QPushButton::clicked, this, &COSEC::saveLogAs);

        QPushButton* openBtn = new QPushButton("Open Log Folder");
        openBtn->setIcon(QIcon::fromTheme("folder-open"));
        openBtn->setMinimumWidth(140);
        connect(openBtn, &QPushButton::clicked, this, &COSEC::openLogFolder);

        btnLayout->addWidget(copyBtn);
        btnLayout->addWidget(saveBtn);
        btnLayout->addWidget(openBtn);
        btnLayout->addStretch();

        layout->addLayout(btnLayout);
        return page;
    }

    inline QWidget* createDetailsPage() {
        QWidget* page = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(page);
        layout->setContentsMargins(15, 15, 15, 15);
        layout->setSpacing(15);

        QHBoxLayout* topLayout = new QHBoxLayout();
        topLayout->setSpacing(20);

        QLabel* iconLabel = new QLabel();
        iconLabel->setPixmap(windowIcon.isNull() ?
                                 QIcon::fromTheme("dialog-error").pixmap(128, 128) :
                                 windowIcon.pixmap(128, 128));
        iconLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        iconLabel->setFixedSize(140, 140);
        topLayout->addWidget(iconLabel);

        QWidget* infoWidget = new QWidget();
        QVBoxLayout* infoLayout = new QVBoxLayout(infoWidget);
        infoLayout->setSpacing(8);
        infoLayout->setContentsMargins(0, 0, 0, 0);

        infoLayout->addWidget(new QLabel("<h3>Application Crashed</h3>"));

        QLabel* sigLabel = new QLabel("<b>Signal:</b> " + QString::fromStdString(crashInfo.signalName));
        sigLabel->setWordWrap(true);
        infoLayout->addWidget(sigLabel);

        QLabel* binLabel = new QLabel("<b>Binary:</b> " + QString::fromStdString(crashInfo.executableName));
        binLabel->setWordWrap(true);
        infoLayout->addWidget(binLabel);

        if (!windowTitle.isEmpty()) {
            QLabel* titleLabel = new QLabel("<b>Window Title:</b> " + windowTitle);
            titleLabel->setWordWrap(true);
            infoLayout->addWidget(titleLabel);
        }

        infoLayout->addStretch();
        topLayout->addWidget(infoWidget, 1);
        layout->addLayout(topLayout);

        layout->addWidget(new QLabel("<b>Session Information</b>"));

        QWidget* detailsWidget = new QWidget();
        QVBoxLayout* detailsLayout = new QVBoxLayout(detailsWidget);
        detailsLayout->setSpacing(8);
        detailsLayout->setContentsMargins(10, 5, 10, 5);

        auto addDetail = [&](const char* label, const QString& value) {
            QLabel* lbl = new QLabel(QString("<b>%1:</b> %2").arg(label).arg(value));
            lbl->setWordWrap(true);
            lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
            detailsLayout->addWidget(lbl);
        };

        addDetail("Started", QString::fromStdString(crashInfo.startTime));
        addDetail("Crashed", QString::fromStdString(crashInfo.timestamp));
        addDetail("Log File", QString::fromStdString(crashInfo.logPath));

        layout->addWidget(detailsWidget);

        QWidget* lcdWidget = new QWidget();
        QVBoxLayout* lcdLayout = new QVBoxLayout(lcdWidget);
        lcdLayout->setAlignment(Qt::AlignCenter);
        lcdLayout->setSpacing(5);

        QLabel* sessLabel = new QLabel("<b>Session Duration:</b>");
        sessLabel->setAlignment(Qt::AlignCenter);
        lcdLayout->addWidget(sessLabel);

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
        lcdLayout->addWidget(lcd);

        QLabel* formatLabel = new QLabel("(HH:MM:SS.CS)");
        formatLabel->setAlignment(Qt::AlignCenter);
        formatLabel->setStyleSheet("color: gray; font-size: 10px;");
        lcdLayout->addWidget(formatLabel);

        layout->addWidget(lcdWidget);
        layout->addStretch();

        return page;
    }

    inline QWidget* createCrashReporterPage() {
        QWidget* page = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(page);
        layout->setContentsMargins(15, 15, 15, 15);
        layout->setSpacing(12);

        QString titleText = windowTitle.isEmpty() ?
                                QString::fromStdString(crashInfo.executableName) + " has crashed" :
                                windowTitle + " has crashed";

        QLabel* title = new QLabel("<h2>" + titleText + "</h2>");
        title->setWordWrap(true);
        layout->addWidget(title);

        QLabel* desc = new QLabel(
            "The application encountered a fatal error and needs to close. "
            "Below is the stack trace that may help identify the issue.");
        desc->setWordWrap(true);
        desc->setStyleSheet("color: #555;");
        layout->addWidget(desc);

        layout->addSpacing(10);
        layout->addWidget(new QLabel("<b>Stack Trace:</b>"));

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
        stackText->setMinimumHeight(200);
        layout->addWidget(stackText, 1);

        QHBoxLayout* btnLayout = new QHBoxLayout();
        btnLayout->setSpacing(10);

        QPushButton* restartBtn = new QPushButton("Restart Application");
        restartBtn->setIcon(QIcon::fromTheme("view-refresh"));
        restartBtn->setMinimumHeight(45);
        restartBtn->setStyleSheet("QPushButton { font-weight: bold; }");
        connect(restartBtn, &QPushButton::clicked, this, &COSEC::restartApplication);

        QPushButton* closeBtn = new QPushButton("Close");
        closeBtn->setIcon(QIcon::fromTheme("window-close"));
        closeBtn->setMinimumHeight(45);
        connect(closeBtn, &QPushButton::clicked, this, &COSEC::closeApplication);

        btnLayout->addWidget(restartBtn);
        btnLayout->addWidget(closeBtn);

        layout->addLayout(btnLayout);
        return page;
    }

private slots:
    inline void saveLogAs() {
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
    }

    inline void openLogFolder() {
        QFileInfo fi(QString::fromStdString(crashInfo.logPath));
        if (!QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absolutePath()))) {
            QMessageBox::warning(this, "Error", "Failed to open log folder.");
        }
    }

    inline void restartApplication() {
        if (!applicationPath.isEmpty()) {
            QProcess::startDetached(applicationPath, QStringList());
            closeApplication();
        } else {
            QMessageBox::warning(this, "Error", "Cannot restart application: path unknown.");
        }
    }

    inline void closeApplication() {
        accept();
        QTimer::singleShot(100, []() {
            QApplication::quit();
            std::exit(0);
        });
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
              << "\nTime: " << crashInfo.timestamp << "\nLog: " << crashInfo.logPath << std::endl;

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
