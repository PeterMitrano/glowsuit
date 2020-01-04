#include <common.h>
#include <midi/midi_file_player.h>
#include <QThread>
#include <iostream>

template<typename TimePoint>
auto to_ms(TimePoint time_point)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(time_point).count();
}

MidiFilePlayer::MidiFilePlayer(QWidget *parent) : QObject(parent)
{
}

MidiFilePlayer::~MidiFilePlayer()
{
    killed = true;
    thread.join();
}

void MidiFilePlayer::parse_midifile()
{
    mutex.lock();
    // TODO: make track number an argument?
    auto const track = midifile[0];
    states_vector.clear();
    // Initial OFF message to turn every thing off
    states_vector.emplace_back(0, State{});
    State current_state;
    auto const size = track.size();
    for (int event_idx = 0; event_idx < size; ++event_idx)
    {
        auto const event = track[event_idx];
        auto const tick = event.tick;

        if (!(event.isNoteOn() || event.isNoteOff()))
        {
            continue;
        }

        {
            auto const data = event.data();
            // why are all of these off by an octave?
            int const bit_idx = static_cast<int>(data[1]) - midi_note_offset + 12 * octave_offset;
            // TODO: something's wrong with my test midi file - this shouldn't be here
            if (bit_idx < 0 || bit_idx >= 6 * 8)
            {
                continue;
            }
            current_state.set(bit_idx, event.isNoteOn());
        }

        // look ahead and merge in any of the next events which are supposed to occur simulatenously
        for (int lookahead_idx = event_idx + 1; lookahead_idx < track.size(); ++lookahead_idx)
        {
            auto const future_event = track[lookahead_idx];
            auto const future_tick = future_event.tick;
            if (future_tick == tick)
            {
                auto const future_data = future_event.data();
                auto const future_bit_idx = (static_cast<int>(future_data[1]) - midi_note_offset + 12 * octave_offset);
                if (future_bit_idx < 0 || future_bit_idx >= 6 * 8)
                {
                    continue;
                }
                current_state.set(future_bit_idx, future_event.isNoteOn());
                event_idx += 1;
            } else
            {
                break;
            }
        }
        // FIXME: instead of tick we store onset
        auto const onset_ms = static_cast<int>(midifile.getTimeInSeconds(tick) * 1000);
        states_vector.emplace_back(onset_ms, current_state);
    }

    mutex.unlock();
}

void MidiFilePlayer::midi_file_changed(QString midi_filename)
{
    midifile.read(midi_filename.toStdString());
    parse_midifile();
}

void MidiFilePlayer::start_thread()
{
    auto func = [&]()
    {
        while (!killed)
        {
            if (!playing)
            {
                continue;
            }

            if (current_state_idx >= states_vector.size())
            {
                continue;
            }

            auto const now = std::chrono::high_resolution_clock::now();
            auto const dt_since_latest_reference_ms = to_ms(now - latest_clock_reference);
            auto const current_time_ms = dt_since_latest_reference_ms + latest_timer_reference_ms;

            // get current onset directory from states_vector
            // check if it's time to emit the next midi event based on the estimated current time
            auto const pair = states_vector[current_state_idx];
            auto const onset_ms = pair.first;
            auto const state = pair.second;
            if (current_time_ms >= onset_ms)
            {
                // visualizer
                emit_to_visualizer(state);

                // transmit
                if (xbee_serial)
                {
                    xbee_serial->write(state.data.data(), message_size);
                }
                ++current_state_idx;
            }
        }
    };
    // start running func in another thread.
    thread = std::thread(func);
}

void MidiFilePlayer::emit_to_visualizer(State const &state)
{
    for (auto suit_idx{0u}; suit_idx < num_suits; ++suit_idx)
    {
        auto const byte = state.data[suit_idx];
        for (auto bit_idx{0u}; bit_idx < 8; ++bit_idx)
        {
            auto const bit = (byte >> bit_idx) & 0x1;
            auto const command = bit == 1 ? 144 : 128;
            auto const channel_number = bit_idx;

            emit midi_event(suit_idx + 1, command, channel_number);
        }
    }

    // transmit
    if (xbee_serial != nullptr)
    {
        xbee_serial->write(state.data.data(), message_size);
    }
}

void MidiFilePlayer::seek(int seconds)
{
    changed(seconds * 1000);
}

void MidiFilePlayer::position_changed(qint64 time_ms)
{
    changed(time_ms);
}

void MidiFilePlayer::changed(qint64 time_ms)
{
    if (states_vector.empty())
    {
        return;
    }

    // restart the timer, start counting from time_ms
    // also reset the index to the midi event which is the next one after time_ms
    latest_timer_reference_ms = time_ms;
    latest_clock_reference = std::chrono::high_resolution_clock::now();
    auto predicate = [&](std::pair<int, State> const &pair)
    {
        auto const onset_ms = pair.first;
        return onset_ms >= time_ms;
    };
    auto find_it = std::find_if(states_vector.begin(), states_vector.end(), predicate);
    if (find_it == states_vector.cend())
    {
        // just ignore this
        return;
    }
    auto const idx = std::distance(states_vector.begin(), find_it);
    if (idx < 0 || idx >= states_vector.size())
    {
        // explode
        throw std::runtime_error("couldn't find a valid state for this time");
    }
    current_state_idx = idx;
}

void MidiFilePlayer::play()
{
    latest_clock_reference = std::chrono::high_resolution_clock::now();
    playing = true;
}

void MidiFilePlayer::pause()
{
    playing = false;
}

void MidiFilePlayer::stop()
{
    current_state_idx = 0;
    playing = false;
}

void MidiFilePlayer::octave_spinbox_changed(int value)
{
    mutex.lock();
    octave_offset = value;
    mutex.unlock();
}
