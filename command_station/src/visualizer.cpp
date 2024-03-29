#include <common.h>
#include <visualizer.h>

#include <QFile>
#include <QMessageBox>
#include <QPainter>
#include <chrono>
#include <fstream>
#include <iostream>

Visualizer::Visualizer(QWidget *parent) : QWidget(parent) { my_resize(); }

int Visualizer::load_suit() {
  if (!QFile::exists("suit.json")) {
    QMessageBox::warning(this, tr("Error"),
                         tr("suit.json was not found, it should be in the same folder as the executable."
                            "You'll need to rerun the program once you've put suit.json in the right location"));
    return 0;
  }
  std::ifstream suit_description_file("suit.json");

  json j;
  suit_description_file >> j;
  suit_description.emplace(j);

  num_channels = suit_description.value()["channels"].size();

  on_channels.resize(num_suits);
  for (auto &suit : on_channels) {
    suit.resize(num_channels);
  }
  return static_cast<int>(num_channels);
}

void Visualizer::paintEvent(QPaintEvent *event) {
  if (!suit_description) {
    return;
  }

  if (!use_visualizer) {
    return;
  }

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  for (auto suit_idx{0}; suit_idx < num_suits; ++suit_idx) {
    int const sx = offset_x + suit_width * suit_idx;
    int const sy = offset_y;
    auto channel_idx{0u};
    for (auto &channel_description : suit_description.value()["channels"]) {
      if (channel_description.contains("lines")) {
        for (auto &line : channel_description["lines"]) {
          auto color = off_color;
          auto color_description = line["color"];
          int r = color_description[0];
          int g = color_description[1];
          int b = color_description[2];
          if (on_channels[suit_idx][channel_idx]) {
            if ((back && line["back"]) || (front && line["front"])) {
              color = QColor(r, g, b, 255);
            }
          }
          QPen pen;
          pen.setColor(color);
          pen.setWidth(get_pen_width());
          painter.setPen(pen);
          auto const x1 = static_cast<int>(scale * (sx + static_cast<int>(line["x1"])));
          auto const y1 = static_cast<int>(scale * (sy + static_cast<int>(line["y1"])));
          auto const x2 = static_cast<int>(scale * (sx + static_cast<int>(line["x2"])));
          auto const y2 = static_cast<int>(scale * (sy + static_cast<int>(line["y2"])));
          painter.drawLine(x1, y1, x2, y2);
        }
      }
      if (channel_description.contains("circles")) {
        for (auto &circle : channel_description["circles"]) {
          auto color = off_color;
          auto color_description = circle["color"];
          int r = color_description[0];
          int g = color_description[1];
          int b = color_description[2];
          if (on_channels[suit_idx][channel_idx]) {
            if ((back && circle["back"]) || (front && circle["front"])) {
              color = QColor(r, g, b, 255);
            }
          }
          QPen pen;
          pen.setColor(color);
          pen.setWidth(get_pen_width());
          painter.setPen(pen);
          auto const x = static_cast<int>(scale * (sx + static_cast<int>(circle["x"])));
          auto const y = static_cast<int>(scale * (sy + static_cast<int>(circle["y"])));
          auto const radius = static_cast<int>(scale * static_cast<int>(circle["r"]));
          painter.drawEllipse(x, y, radius, radius);
        }
      }
      ++channel_idx;
    }
  }
}

void Visualizer::front_status_clicked(bool const checked) { front = !front; }

void Visualizer::back_status_clicked(bool const checked) { back = !back; }

void Visualizer::on_live_midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number) {
  if (!viz_from_live_midi) {
    return;
  }

  generic_on_midi_event(suit_number, command, channel_number);
}

void Visualizer::on_midi_file_event(unsigned int suit_number, unsigned int command, unsigned int channel_number) {
  if (viz_from_live_midi) {
    return;
  }

  generic_on_midi_event(suit_number, command, channel_number);
}

void Visualizer::generic_on_midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number) {
  if (suit_number < 1 || suit_number > num_suits || channel_number < 0 || channel_number > 7) {
    return;
  }
  if (command == midi_note_off) {
    on_channels[suit_number - 1][channel_number] = false;
  } else if (command == midi_note_on) {
    on_channels[suit_number - 1][channel_number] = true;
  } else {
    // just ignore any other types of midi messages
    return;
  }
  update();
}

int Visualizer::get_pen_width() const { return std::max(1, static_cast<int>(scale * 2)); }

void Visualizer::viz_scale_changed(double viz_scale) {
  this->scale = viz_scale;
  my_resize();
  update();
}

void Visualizer::my_resize() {
  auto const width = static_cast<int>(scale * num_suits * suit_width + offset_x * 2);
  setMinimumSize(width, static_cast<int>(scale * base_height));
}

void Visualizer::use_visualizer_checked(bool checked) { use_visualizer = checked; }
