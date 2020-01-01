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

public slots:
	// TODO: use checkable QPushButtons instead of my own booleans
	void front_status_clicked();

	void back_status_clicked();

public:

	Ui_MainWindow ui;
	Visualizer* viz;
	QString button_off_style;
	QString button_on_style;
};
