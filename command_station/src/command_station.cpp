// Windows is a special header , it must go first
#ifdef WIN32
#include <Windows.h>
#endif

#include <QApplication>
#include <QMainWindow>
#include <QtPlugin>
#include <QFile>

#include <json.h>
#include <main_widget.h>
#include <suit_common.h>

using json = nlohmann::json;

using namespace std::chrono_literals;

int main(int argc, char **argv)
{
    qRegisterMetaType<std::vector<uint8_t> >();
    qRegisterMetaType<SuitCommand>();
    qRegisterMetaType<Data>();

    QApplication app(argc, argv);

    QFile styleFile(":/style.qss");
    styleFile.open(QFile::ReadOnly);

    QString style(styleFile.readAll());
    app.setStyleSheet(style);

    MainWidget main_widget;
    main_widget.show();
    return app.exec();
}
