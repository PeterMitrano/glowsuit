#pragma once

#include <optional>
#include <stdint.h>

#include <QColor>
#include <QWidget>

#include <json.h>

using json = nlohmann::json;

class Visualizer : public QWidget
{
	Q_OBJECT

public:
	Visualizer(std::optional<json> const& suit_description, size_t num_channels, QWidget* parent = nullptr);

	unsigned int offset_x = 10;
	unsigned int offset_y = 10;
	unsigned int suit_width = 100;
	unsigned int width = 0;
	unsigned int height = 200;
	unsigned int left = 1366 - width - 10;
	unsigned int top = 10;
	QColor off_color{ 50, 50, 50, 255 };
	unsigned int num_channels = 0;
	std::vector<std::vector<bool>> on_channels;
	bool front = true;
	bool back = true;
	std::optional<json> suit_description;
	bool viz_from_live_midi{ false };

public slots:
	void front_status_clicked(bool const checked);

	void back_status_clicked(bool const checked);

	void on_midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number);

protected:
	void paintEvent(QPaintEvent* event) override;


};
