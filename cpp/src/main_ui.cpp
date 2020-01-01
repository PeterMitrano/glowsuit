#include <main_ui.h>

MainUI::MainUI(Ui_MainWindow const ui, Visualizer* const viz)
	: ui(ui),
	viz(viz),
	button_off_style(" QPushButton { background-color: #e83535; color: #000000 } QPushButton:pressed { background-color: #ad1010; color: #000000 }"),
	button_on_style(" QPushButton { background-color: #58eb34; color: #000000 } QPushButton:pressed { background-color: #34ba13; color: #000000 }")
{
}

void MainUI::setup_ui()
{
	ui.front_button->setStyleSheet(button_on_style);
	ui.back_button->setStyleSheet(button_on_style);

	QObject::connect(ui.front_button, &QPushButton::clicked, this, &MainUI::front_status_clicked);
	QObject::connect(ui.back_button, &QPushButton::clicked, this, &MainUI::back_status_clicked);
}

void MainUI::front_status_clicked()
{
	viz->front_status_clicked();
	if (viz->front)
	{
		ui.front_button->setStyleSheet(button_on_style);
	}
	else
	{
		ui.front_button->setStyleSheet(button_off_style);
	}
}

void MainUI::back_status_clicked()
{
	viz->back_status_clicked();
	if (viz->back)
	{
		ui.back_button->setStyleSheet(button_on_style);
	}
	else {
		ui.back_button->setStyleSheet(button_off_style);
	}
}
