// Windows is a special header , it must go first
#include <Windows.h>

#include <bitset>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>

#include <QApplication>
#include <qmainwindow.h>
#include <QPushButton>
#include <QtPlugin>

// TODO: fix this path?
#include <../src/ui_mainwindow.h>

#include <json.h>
#include <visualizer.h>
#include <main_ui.h>

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);

using json = nlohmann::json;

using namespace std::chrono_literals;

std::optional<json> load_suit_description();

int main(int argc, char** argv)
{
	auto const suit_description = load_suit_description();
	auto const num_channels = [&]() {
		if (suit_description) {
			return suit_description.value()["channels"].size();
		}
		// Default size is 6
		return 6u;
	}();

	QApplication app(argc, argv);
	QMainWindow main_window;
	Ui_MainWindow ui;
	ui.setupUi(&main_window);
	Visualizer viz(suit_description, num_channels);
	ui.verticalLayout->addWidget(&viz);
	// TODO: use unique_ptr for Viz here
	MainUI main(ui, &viz, num_channels);
	main.setup_ui();
	main_window.show();
	return app.exec();
}



std::optional<json> load_suit_description()
{
	std::ifstream suit_description_file("suit.json");
	if (!suit_description_file.fail()) {
		// use QT dialog to show error?
		json suit_description;
		suit_description_file >> suit_description;
		return std::optional<json>(suit_description);
	}
	return std::optional<json>{};
}

