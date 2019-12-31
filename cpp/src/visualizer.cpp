#include <QPainter>

#include <visualizer.h>

Visualizer::Visualizer(std::optional<json> const& suit_description, size_t const num_channels, QWidget* parent)
	: QWidget(parent),
	suit_description(suit_description),
	num_channels(num_channels)

{

	width = num_suits * suit_width + offset_x * 2;
	setMinimumSize(width, height);
}

void Visualizer::paintEvent(QPaintEvent* event)
{
	QPainter painter;
	painter.setRenderHint(QPainter::Antialiasing);
	for (auto suit_idx{ 0u }; suit_idx <= num_suits; ++suit_idx)
	{
		//sx = self.offset_x + self.suit_width * suit_idx
		//	sy = self.offset_y
		//	for channel_idx, channel_description in enumerate(self.suit['channels']) :
		//		if 'lines' in channel_description :
		//for line in channel_description['lines'] :
		//	color = self.off_color
		//	r, g, b = line['color']
		//	if self.on_channels[suit_idx, channel_idx] :
		//		if (self.back and line['back']) or (self.front and line['front']) :
		//			color = QColor(r, g, b, 255)
		//			painter.setPen(color)
		//			painter.drawLine(sx + line['x1'], sy + line['y1'], sx + line['x2'], sy + line['y2'])

		//			if 'circles' in channel_description :
		//for circle in channel_description['circles'] :
		//	color = self.off_color
		//	r, g, b = circle['color']
		//	if self.on_channels[suit_idx, channel_idx] :
		//		if (self.back and circle['back']) or (self.front and circle['front']) :
		//			color = QColor(r, g, b, 255)
		//			painter.setPen(color)
		//			painter.drawEllipse(sx + circle['x'], sy + circle['y'], circle['r'], circle['r'])
	}
}
