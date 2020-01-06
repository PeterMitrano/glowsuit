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
    Visualizer(QWidget *parent = nullptr);

    int offset_x = 10;
    int offset_y = 10;
    int suit_width = 100;
    int const base_height = 200;
    QColor off_color{45, 48, 51, 255};
    unsigned int num_channels = 0;
    std::vector<std::vector<bool>> on_channels;
    bool front = true;
    bool back = true;
    double scale{1.0};
    std::optional<json> suit_description;
    bool viz_from_live_midi{false};

    int load_suit();

    [[nodiscard]] int get_pen_width() const;

    void my_resize();

public slots:

    void viz_scale_changed(double viz_scale);

    void front_status_clicked(bool checked);

    void back_status_clicked(bool checked);

    void on_live_midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number);

    void on_midi_file_event(unsigned int suit_number, unsigned int command, unsigned int channel_number);

    void generic_on_midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number);

protected:
    void paintEvent(QPaintEvent *event) override;

};
