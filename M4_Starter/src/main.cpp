#include "aiws/gui/main_window.hpp"
#include <unistd.h>
#include <QApplication>
#include <QMessageBox>
#include <QCommandLineParser>

// TODO: Define the ExitCallback function used to confirm an Exit request.
void confirm_exit(QWidget* parent)
{
    const auto answer = QMessageBox::question(parent, "Exit", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No);
    if (answer == QMessageBox::Yes)
    {
        auto* window = static_cast<aiws::gui::MainWindow*>(parent);

        window->request_shutdown();
    }
}
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();

    QCommandLineOption delay_option("build-delay-ms", "Extra build delay per document in milliseconds.", "milliseconds", "0");
    QCommandLineOption socket_option("ipc-socket", "Unix domain socket path.", "path", "/tmp/aiws-" + QString::number(::getuid()) + ".sock");

    parser.addOption(delay_option);
    parser.addOption(socket_option);
    parser.process(app);

    bool valid = false;
    int delay = parser.value(delay_option).toInt(&valid);

    if (!valid || delay < 0)
    {
        return 1;
    }

    aiws::gui::MainWindow window;
    window.set_exit_callback(&confirm_exit);
    window.set_build_delay_ms(delay);
    if (!window.start_ipc_server(parser.value(socket_option)))
    {
        return 1;
    }
    window.show();
    return app.exec();
}
