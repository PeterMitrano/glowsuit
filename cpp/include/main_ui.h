#pragma once

#include <string>

#include <QObject>
#include <QString>

#include <../src/ui_mainwindow.h>

#include <visualizer.h>

class MainUI : public QObject
{
	Q_OBJECT

public:

	/// viz must not be null
	MainUI(Ui_MainWindow ui, Visualizer* viz);

	void setup_ui();

public:

	Ui_MainWindow ui;
	Visualizer* viz;
};
