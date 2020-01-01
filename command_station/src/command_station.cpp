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
#include <QMessageBox>
#include <QPushButton>
#include <QtPlugin>
#include <QFile>

// TODO: fix this path?
#include <../src/ui_mainwindow.h>

#include <json.h>
#include <visualizer.h>
#include <my_main_window.h>

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);

using json = nlohmann::json;

using namespace std::chrono_literals;

std::optional<json> load_suit_description();


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

	Visualizer viz(suit_description, num_channels);

	Ui_MainWindow ui;
	ui.verticalLayout->addWidget(&viz);
	// TODO: use unique_ptr for Viz here
	MyMainWindow main_window(ui, &viz, num_channels);
	ui.setupUi(&main_window);
	main_window.setup_ui();
	main_window.show();
	return app.exec();
}



std::optional<json> load_suit_description()
{
	if (!QFile::exists("suit.json"))
	{
		QMessageBox suit_description_message_box;
		suit_description_message_box.setText("suit.json was not found, it should be in the same folder as the executable.");
		suit_description_message_box.exec();
		return std::optional<json>{};
	}
	std::ifstream suit_description_file("suit.json");

	json suit_description;
	suit_description_file >> suit_description;
	return std::optional<json>(suit_description);
}
