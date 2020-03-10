#include <QThread>
#include <iostream>

#include <midi/midi_file_player.h>
#include <common.h>

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

    all_events.clear();
    // Initial OFF message to turn every thing off
    all_events.emplace_back(0, State{});
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
        all_events.emplace_back(onset_ms, current_state);
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
    /** Alternative plan:
     *  - every 100ms, second the next 100 choreo events
     *  - time is included in each message so suits can stay synced include constant offset to account for latency?)
     */
    auto func = [&]()
    {
        Window states_window;

        while (!killed)
        {
            if (!playing)
            {
                continue;
            }

            if (current_event_idx >= all_events.size())
            {
                continue;
            }

            current_song_time_ms = current_song_time(latest_clock_reference, latest_song_time_reference);

            // check the next onsets until we find the event which is about to come next

            // Create window of the next 100 events
            states_window.current_song_time = current_song_time_ms;
            for (auto window_idx{0u}; window_idx < WindowSize; ++window_idx)
            {
                auto const state_vector_idx = current_event_idx + window_idx;
                // only fill as much events as we actuall have. The rest will be zeros, by default
                if (state_vector_idx < all_events.size())
                {
                    states_window.events[window_idx] = all_events[state_vector_idx];
                }
            }
            emit_to_xbee(states_window);

            // visualizer has perfect "communication" so we just call it when necessary
            auto const current_state = all_events[current_event_idx];
            if (current_song_time_ms >= current_state.onset_ms)
            {
                emit_to_visualizer(current_state.state);
                ++current_event_idx;
            }
            QThread::msleep(100);
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
}

void MidiFilePlayer::emit_to_xbee(Window const &states_window)
{
    if (xbee_serial)
    {
        try
        {
            std::vector<uint8_t> window_msg;
            auto const time_bytes = to_bytes<int>(states_window.current_song_time);
            std::copy(time_bytes.cbegin(), time_bytes.cend(), window_msg.end());
            auto const size_bytes = to_bytes<int>(WindowSize);
            std::copy(size_bytes.cbegin(), size_bytes.cend(), window_msg.end());
            for (auto const &event : states_window.events)
            {
                auto const onset_bytes = to_bytes<unsigned int>(event.onset_ms);
                std::copy(onset_bytes.cbegin(), onset_bytes.cend(), window_msg.end());
                std::copy(event.state.data.cbegin(), event.state.data.cend(), window_msg.end());
            }

            xbee_serial->write(window_msg.data(), BytesPerMessage * WindowSize);
        } catch (serial::SerialException)
        {
            // pass
        }
    }
}

void MidiFilePlayer::emit_to_xbee(State const &state)
{
    if (xbee_serial)
    {
        try
        {
            xbee_serial->write(state.data.data(), BytesPerMessage);
        } catch (serial::SerialException)
        {
            // pass
        }
    }
}

void MidiFilePlayer::seek(int time_ms)
{
    if (all_events.empty())
    {
        return;
    }

    // restart the timer, start counting from time_ms
    // also reset the index to the midi event which is the next one after time_ms
    latest_song_time_reference = time_ms;
    latest_clock_reference = std::chrono::high_resolution_clock::now();
    auto predicate = [&](Event const &onset_state)
    {
        auto const onset_ms = onset_state.onset_ms;
        return onset_ms >= time_ms;
    };
    auto find_it = std::find_if(all_events.begin(), all_events.end(), predicate);
    if (find_it == all_events.cend())
    {
        // just ignore this
        return;
    }
    auto const idx = std::distance(all_events.begin(), find_it);
    if (idx < 0 || idx >= all_events.size())
    {
        // explode
        throw std::runtime_error("couldn't find a valid state for this time");
    }
    current_event_idx = idx;
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

void MidiFilePlayer::status_changed(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia)
    {
        stop();
    }
}
