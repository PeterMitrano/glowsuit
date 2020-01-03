#include <Windows.h>
#include <common.h>
#include <midi/midi_file_player.h>
#include <QThread>


MidiFilePlayer::MidiFilePlayer(QWidget* parent) : QObject(parent)
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
    // Initial OFF message to turn every thing off
    states_vector.clear();
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
            auto const bit_idx = (static_cast<int>(data[1]) - midi_note_offset + 12 * octave_offset);
            // TODO: something's wrong with my test midi file - this shouldn't be here
            if (bit_idx < 0)
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
                if (future_bit_idx < 0)
                {
                    continue;
                }
                current_state.set(future_bit_idx, future_event.isNoteOn());
                event_idx += 1;
            }
            else
            {
                break;
            }
        }
        states_vector.emplace_back(tick, current_state);
    }

    // FInal OFF message to turn every thing off
    //auto const final_tick = track[size - 1].tick + 1;
    //states_vector.emplace_back(final_tick, State{});
    mutex.unlock();
}

void MidiFilePlayer::midi_file_changed(QString midi_filename)
{
    midifile.read(midi_filename.toStdString());
    parse_midifile();
}

void MidiFilePlayer::start_thread()
{
    //auto func = [&]() {
    //    while (!killed)
    //    {
    //        QThread::msleep(50);
        //}
    //};
    // start running func in another thread.
    //thread = std::thread(func);
}

#include <sstream>
void MidiFilePlayer::emit_to_visualizer(State const& state)
{
    for (auto suit_idx{ 0u }; suit_idx < num_suits; ++suit_idx) {
        auto const byte = state.data[suit_idx];
        for (auto bit_idx{ 0u }; bit_idx < 8; ++bit_idx)
        {
            auto const bit = (byte >> bit_idx) & 0x1;
            auto const command = bit == 1 ? 144 : 128;
            auto const channel_number = bit_idx;

            emit midi_event(suit_idx + 1, command, channel_number);
        }
    }

    // transmit
    if (xbee_serial != nullptr) {
        xbee_serial->write(state.data.data(), message_size);
    }
}

void MidiFilePlayer::position_changed(qint64 time_ms)
{
    if (!playing)
    {
        return;
    }

    for (auto current_state_idx = 0u; current_state_idx <= states_vector.size(); ++current_state_idx)
        //auto current_state_idx = 0u;
        //for (auto const& pair : states_vector)
    {
        auto const pair = states_vector[current_state_idx];
        auto const tick = pair.first;
        auto const state = pair.second;
        char buff[100];
        snprintf(buff, 100, "%d: %x %x %x %x\n", current_state_idx, state.data[0], state.data[1], state.data[2], state.data[3], state.data[4], state.data[5]);
        OutputDebugString(buff);
        auto const onset_ms = static_cast<int>(midifile.getTimeInSeconds(tick) * 1000);
        if (onset_ms >= time_ms)
        {
            emit_to_visualizer(state);
            break;
        }
        ++current_state_idx;
    }

}

void MidiFilePlayer::play()
{
    playing = true;
}

void MidiFilePlayer::pause()
{
    playing = false;
}

void MidiFilePlayer::stop()
{
    playing = false;
}

void MidiFilePlayer::octave_spinbox_changed(int value)
{
    mutex.lock();
    octave_offset = value;
    mutex.unlock();
}
