#include <QPainter>

#include <visualizer.h>
#include <common.h>

Visualizer::Visualizer(std::optional<json> const &suit_description, size_t const num_channels, QWidget *parent)
        : QWidget(parent),
          suit_description(suit_description),
          num_channels(num_channels)
{
    width = num_suits * suit_width + offset_x * 2;
    on_channels.resize(num_suits);
    for (auto &suit : on_channels)
    {
        suit.resize(num_channels);
    }
    setMinimumSize(width, height);
}

void Visualizer::paintEvent(QPaintEvent *event)
{
    if (!suit_description)
    {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (auto suit_idx{0}; suit_idx < num_suits; ++suit_idx)
    {
        int const sx = offset_x + suit_width * suit_idx;
        int const sy = offset_y;
        auto channel_idx{0u};
        for (auto &channel_description : suit_description.value()["channels"])
        {
            if (channel_description.contains("lines"))
            {
                for (auto &line : channel_description["lines"])
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
                    QPen pen;
                    pen.setColor(color);
                    pen.setWidth(2);
                    painter.setPen(pen);
                    painter.drawLine(sx + static_cast<int>(line["x1"]),
                                     sy + static_cast<int>(line["y1"]),
                                     sx + static_cast<int>(line["x2"]),
                                     sy + static_cast<int>(line["y2"]));
                }
            }
            if (channel_description.contains("circles"))
            {
                for (auto &circle : channel_description["circles"])
                {
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
                    QPen pen;
                    pen.setColor(color);
                    pen.setWidth(2);
                    painter.setPen(pen);
                    painter.drawEllipse(sx + static_cast<int>(circle["x"]),
                                        sy + static_cast<int>(circle["y"]),
                                        circle["r"],
                                        circle["r"]);
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

void Visualizer::back_status_clicked(bool const checked)
{
    back = !back;
}

void Visualizer::on_live_midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number)
{
    if (!viz_from_live_midi)
    {
        return;
    }

    generic_on_midi_event(suit_number, command, channel_number);
}

void Visualizer::on_midi_file_event(unsigned int suit_number, unsigned int command, unsigned int channel_number)
{
    if (viz_from_live_midi)
    {
        return;
    }

    generic_on_midi_event(suit_number, command, channel_number);
}

void Visualizer::generic_on_midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number)
{
    if (suit_number < 1 || suit_number > num_suits || channel_number < 0 || channel_number > 7)
    {
        return;
    }
    if (command == 128)
    {

        on_channels[suit_number - 1][channel_number] = 0;
    } else if (command == 144)
    {
        on_channels[suit_number - 1][channel_number] = 1;
    } else
    {
        // just ignore any other types of midi messages
        return;
    }
    update();
}
