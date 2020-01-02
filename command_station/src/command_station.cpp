// Windows is a special header , it must go first
#include <Windows.h>

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

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);

using json = nlohmann::json;

using namespace std::chrono_literals;


int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	auto const suit_description = load_suit_description();
	auto const num_channels = [&]() {
		if (suit_description) {
			return suit_description.value()["channels"].size();
		}
		// Default size is 6
		return 6u;
	}();

	MainWidget main_widget(suit_description, num_channels);
	main_widget.show();
	return app.exec();
}
