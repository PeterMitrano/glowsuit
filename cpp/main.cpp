#include <midi/MidiFile.h>
#include <iostream>

#include <args.h>
#include <thread>
#include <alsa/asoundlib.h>
#include <bitset>
#include <chrono>
#include <cstdlib>
#include <serial/serial.h>
#include <byteset.h>
#include <map>

using namespace std::chrono_literals;

constexpr int message_size = 6;
constexpr int midi_note_offset = 60;
using State = Byteset<message_size>;

void play_music(std::string const &filename)
{
    int pcm;
    unsigned int tmp;
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames;
    char *buff;
    unsigned long buff_size;

    auto rate = 44100u;

    auto const fd = open(filename.c_str(), 'r');

    /* Open the PCM device in playback mode */
    pcm = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (pcm < 0)
    {
        std::cout << "ERROR: Can't open default device. " << snd_strerror(pcm) << '\n';
    }

    /* Allocate parameters object and fill it with default values*/
    snd_pcm_hw_params_alloca(&params);

    snd_pcm_hw_params_any(pcm_handle, params);

    /* Set parameters */
    pcm = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (pcm < 0)
    {
        std::cout << "ERROR: Can't set interleaved mode." << snd_strerror(pcm) << '\n';
    }

    pcm = snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
    if (pcm < 0)
    {
        std::cout << "ERROR: Can't set format." << snd_strerror(pcm) << '\n';
    }

    snd_pcm_hw_params_set_channels(pcm_handle, params, 2);
    pcm = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, nullptr);
    if (pcm < 0)
    {
        std::cout << "ERROR: Can't set rate." << snd_strerror(pcm) << '\n';
    }

    /* Write parameters */
    pcm = snd_pcm_hw_params(pcm_handle, params);
    if (pcm < 0)
    {
        printf("ERROR: Can't set harware parameters. %s\n", snd_strerror(pcm));
    }

    snd_pcm_hw_params_get_channels(params, &tmp);

    snd_pcm_hw_params_get_rate(params, &tmp, nullptr);

    /* Allocate buffer to hold single period */
    snd_pcm_hw_params_get_period_size(params, &frames, nullptr);

    buff_size = frames * 2 * 2;
    buff = (char *) malloc(buff_size);

    snd_pcm_hw_params_get_period_time(params, &tmp, nullptr);

    while (true)
    {

        auto const bytes_read = read(fd, buff, buff_size);
        if (bytes_read == 0)
        {
            break;
        }

        auto const write_result = snd_pcm_writei(pcm_handle, buff, frames);
        if (write_result == -EPIPE)
        {
            std::cout << "XRUN.\n";
            snd_pcm_prepare(pcm_handle);
        } else if (pcm < 0)
        {
            std::cout << "ERROR. Can't write to PCM device. " << snd_strerror(pcm) << "\n";
        }

    }

    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    free(buff);
}

int main(int const argc, char const *const *const argv)
{
    // Argument Parsing
    args::ArgumentParser parser("Plays a Suit MIDI Choreo.");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::CompletionFlag completion(parser, {"complete"});
    args::Positional<std::string> midi_filename_arg(parser, "midi_file", "midi file", args::Options::Required);
    args::Positional<std::string> serial_port_arg(parser, "serial_port", "serial port", args::Options::Required);
    args::Positional<std::string> music_filename_arg(parser, "music_file", "music file", args::Options::Required);

    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (const args::Completion &e)
    {
        std::cout << e.what();
        return 0;
    }
    catch (const args::Help &)
    {
        std::cout << parser;
        return 0;
    }
    catch (const args::ParseError &e)
    {
        std::cerr << e.what() << '\n';
        std::cerr << parser;
        return 1;
    }

    auto const midi_filename = midi_filename_arg.Get();
    auto const music_filename = music_filename_arg.Get();
    auto const serial_port = serial_port_arg.Get();

    // Create serial port
    serial::Serial my_serial(serial_port, 57600, serial::Timeout::simpleTimeout(1000));

    // Parse MIDI file into sequence of serial messages
    smf::MidiFile midifile;
    auto const status = midifile.read(midi_filename);

    if (not status)
    {
        std::cout << "ERROR: couldn't open file\n";
        return EXIT_FAILURE;
    }

    // TODO: make track number an argument
    auto const track = midifile[0];
    std::map<unsigned int, State> states;
    // Initial OFF message to turn every thing off
    states.emplace(0, State{});
    State current_state;
    auto const size = track.size();
    for (int event_idx = 0; event_idx < size; ++event_idx)
    {
        auto const event = track[event_idx];
        auto const tick = event.tick;

        if (not(event.isNoteOn() or event.isNoteOff()))
        {
            continue;
        }

        {
            auto const data = event.data();
            auto const id_number = (static_cast<int>(data[1]) - midi_note_offset);
            current_state.set(id_number, event.isNoteOn());
            // TODO: something's wrong with my test midi file - this shouldn't be here
            if (id_number < 0)
            {
                continue;
            }
        }

        // look ahead and merge in any of the next events which are supposed to occur simulatenously
        for (int lookahead_idx = event_idx + 1; lookahead_idx < track.size(); ++lookahead_idx)
        {
            auto const future_event = track[lookahead_idx];
            auto const future_tick = future_event.tick;
            if (future_tick == tick)
            {
                auto const future_data = future_event.data();
                auto const future_id_number = (static_cast<int>(future_data[1]) - midi_note_offset);
                if (future_id_number < 0)
                {
                    continue;
                }
                current_state.set(future_id_number, future_event.isNoteOn());
                event_idx += 1;
            } else
            {
                break;
            }
        }
        states[tick] = current_state;
    }

    // FInal OFF message to turn every thing off
    states.emplace(track[size - 1].tick + 1, State{});

    // Start the music
    auto thread_func = [&]()
    {
        play_music(music_filename);
    };
    std::thread music_thread(thread_func);

    // Transmit messages
    auto const t0 = std::chrono::high_resolution_clock::now();
    for (auto &pair: states)
    {
        auto const &tick = pair.first;
        auto const &onset_ms = static_cast<int>(midifile.getTimeInSeconds(tick) * 1000);
        auto const &state = pair.second;

        auto const t_last = std::chrono::high_resolution_clock::now();
        while (true)
        {
            auto const now = std::chrono::high_resolution_clock::now();
            auto const dt = now - t0;
            auto const dt_to_last = now - t_last;
            if (dt >= std::chrono::milliseconds(onset_ms))
            {
                // transmit
                my_serial.write(state.data.data(), message_size);
                break;
            }
        }

    }

    music_thread.join();
    return EXIT_SUCCESS;
}
