#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateTime>
#include <QMessageBox>

#include "tan.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow() {
        setWindowTitle("tan test");
        resize(600, 400);

        QWidget* central = new QWidget(this);
        QVBoxLayout* layout = new QVBoxLayout(central);

        QLabel* label = new QLabel("Command:");
        cmdInput = new QLineEdit();
        cmdInput->setPlaceholderText("Enter command");

        QPushButton* btnPath = new QPushButton("term_bin_path");
        QPushButton* btnRun = new QPushButton("run");
        QPushButton* btnSu = new QPushButton("runsu");
        QPushButton* btnClear = new QPushButton("Clear Log");

        logOutput = new QTextEdit();
        logOutput->setReadOnly(true);
        logOutput->setStyleSheet("font-family: monospace;");

        layout->addWidget(label);
        layout->addWidget(cmdInput);
        layout->addWidget(btnPath);
        layout->addWidget(btnRun);
        layout->addWidget(btnSu);
        layout->addWidget(btnClear);
        layout->addWidget(logOutput);

        setCentralWidget(central);

        connect(btnPath, &QPushButton::clicked, this, &MainWindow::testTermPath);
        connect(btnRun, &QPushButton::clicked, this, &MainWindow::testTanrun);
        connect(btnSu, &QPushButton::clicked, this, &MainWindow::testTanrunsu);
        connect(btnClear, &QPushButton::clicked, logOutput, &QTextEdit::clear);
    }

private slots:
    void testTermPath() {
        std::string path = TAN::term_bin_path();
        if (path.empty()) {
            log("term_bin_path: No terminal found");
        } else {
            log("term_bin_path: " + QString::fromStdString(path));
        }
    }

    void testTanrun() {
        QString cmd = cmdInput->text().trimmed();
        if (cmd.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please enter a command");
            return;
        }

        bool result = TAN::tanrun(cmd.toStdString());
        log(QString("tanrun(%1): %2").arg(cmd, result ? "OK" : "FAILED"));
        if (!result) {
            log("  Reason: Terminal not available or command failed");
        }
    }

    void testTanrunsu() {
        QString cmd = cmdInput->text().trimmed();
        if (cmd.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please enter a command");
            return;
        }

        bool result = TAN::tanrunsu(cmd.toStdString());
        log(QString("tanrunsu(%1): %2").arg(cmd, result ? "OK" : "FAILED"));
        if (!result) {
            log("  Reason: Terminal not available or insufficient permissions");
        }
    }

    void log(const QString& msg) {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        logOutput->append(timestamp + " " + msg);
    }

private:
    QLineEdit* cmdInput;
    QTextEdit* logOutput;
};

#include "test.moc"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWindow win;
    win.show();
    return app.exec();
}
