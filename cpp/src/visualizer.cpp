#include <QPainter>

#include <visualizer.h>

Visualizer::Visualizer(std::optional<json> const& suit_description, size_t const num_channels, QWidget* parent)
	: QWidget(parent),
	suit_description(suit_description),
	num_channels(num_channels)

{
	width = num_suits * suit_width + offset_x * 2;
	on_channels.resize(num_suits);
	for (auto& suit : on_channels)
	{
		suit.resize(num_channels);
	}
	setMinimumSize(width, height);
}

void Visualizer::paintEvent(QPaintEvent* event)
{
	if (!suit_description) {
		return;
	}

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	for (auto suit_idx{ 0u }; suit_idx < num_suits; ++suit_idx)
	{
		auto const sx = offset_x + suit_width * suit_idx;
		auto const sy = offset_y;
		auto channel_idx{ 0u };
		for (auto& channel_description : suit_description.value()["channels"])
		{
			if (channel_description.contains("lines"))
			{
				for (auto& line : channel_description["lines"])
				{
					auto color = off_color;
					auto color_description = line["color"];
					auto r = color_description[0];
					auto b = color_description[1];
					auto g = color_description[2];
					if (on_channels[suit_idx][channel_idx])
					{
						if ((back && line["back"]) || (front && line["front"]))
						{
							color = QColor(r, g, b, 255);
						}
					}
					painter.setPen(color);
					painter.drawLine(sx + line["x1"], sy + line["y1"], sx + line["x2"], sy + line["y2"]);
				}
			}
			if (channel_description.contains("circles"))
			{
				for (auto& circle : channel_description["circles"]) {
					auto color = off_color;
					auto color_description = circle["color"];
					auto r = color_description[0];
					auto b = color_description[1];
					auto g = color_description[2];
					if (on_channels[suit_idx][channel_idx])
					{
						if ((back && circle["back"]) || (front && circle["front"]))
						{
							color = QColor(r, g, b, 255);
						}
					}
					painter.setPen(color);
					painter.drawEllipse(sx + circle["x"], sy + circle["y"], circle["r"], circle["r"]);
				}
			}
			++channel_idx;
		}
	}
}

void Visualizer::front_status_clicked(bool const checked)
{
	front = !front;
}

void Visualizer::back_status_clicked(bool const checked) {
	back = !back;
}
