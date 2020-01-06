#include <common.h>
#include <midi/midi_file_player.h>
#include <QThread>
#include <iostream>

template<typename T>
auto to_ms(T time_point)
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

void MidiFilePlayer::parse_track()
{
    mutex.lock();

    auto const track = midifile[track_number];
    auto const event_count = track.getEventCount();
    emit event_count_changed(event_count);

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
        auto const onset_ms = static_cast<int>(midifile.getTimeInSeconds(tick) * 1000);
        states_vector.emplace_back(onset_ms, current_state);
    }

    mutex.unlock();
}

void MidiFilePlayer::midi_file_changed(QString const midi_filename)
{
    midifile.read(midi_filename.toStdString());
    auto const num_tracks = midifile.getNumTracks();
    emit num_tracks_changed(num_tracks);
    if (track_number >= num_tracks - 1)
    {
        track_number = 0;
    }
    parse_track();
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

            current_song_time_ms = current_song_time(latest_clock_reference, latest_song_time_reference);

            // check if it's time to emit the next midi event based on the estimated current time
            auto const pair = states_vector[current_state_idx];
            auto const onset_ms = pair.first;
            if (current_song_time_ms >= onset_ms)
            {

                // visualizer
                auto const state = pair.second;
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
            auto const command = bit == 1 ? midi_note_on : midi_note_off;
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

void MidiFilePlayer::seek(int time_ms)
{
    if (states_vector.empty())
    {
        return;
    }

    // restart the timer, start counting from time_ms
    // also reset the index to the midi event which is the next one after time_ms
    latest_song_time_reference = time_ms;
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
    // FIXME: this API sucks
    current_song_time_ms = current_song_time(latest_clock_reference, latest_song_time_reference);
}

void MidiFilePlayer::play()
{
    latest_song_time_reference = current_song_time_ms;
    latest_clock_reference = std::chrono::high_resolution_clock::now();
    current_song_time_ms = current_song_time(latest_clock_reference, latest_song_time_reference);
    playing = true;
}

void MidiFilePlayer::pause()
{
    playing = false;
}

void MidiFilePlayer::stop()
{
    playing = false;
    seek(0);
}

void MidiFilePlayer::octave_spinbox_changed(int value)
{
    mutex.lock();
    octave_offset = value;
    mutex.unlock();
}

qint64 current_song_time(TimePoint const clock_reference, qint64 const song_time_reference)
{
    auto const now = std::chrono::high_resolution_clock::now();
    auto const dt_since_latest_reference_ms = to_ms(now - clock_reference);
    return dt_since_latest_reference_ms + song_time_reference;
}


void MidiFilePlayer::track_changed(int const value)
{
    track_number = value;
    parse_track();
}
