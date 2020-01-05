// Windows is a special header , it must go first
#ifdef WIN32
#include <Windows.h>
#endif

#include <bitset>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QtPlugin>
#include <QFile>

#include <json.h>
#include <visualizer.h>
#include <main_widget.h>

#ifdef win32
// TODO: is this still really nessecary?
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif

using json = nlohmann::json;

using namespace std::chrono_literals;


int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QFile styleFile(":/style.qss");
    styleFile.open(QFile::ReadOnly);

    QString style(styleFile.readAll());
    app.setStyleSheet(style);

    MainWidget main_widget;
    main_widget.show();
    return app.exec();
}
