#include <Windows.h>

#include <main_ui.h>
#include <QFile>

MainUI::MainUI(Ui_MainWindow const ui, Visualizer* const viz)
	: ui(ui),
	viz(viz)
{
}

#include <iostream>
#include <QDirIterator>

void MainUI::setup_ui()
{
	QObject::connect(ui.front_button, &QPushButton::clicked, viz, &Visualizer::front_status_clicked);
	QObject::connect(ui.back_button, &QPushButton::clicked, viz, &Visualizer::back_status_clicked);
}
