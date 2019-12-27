#include <midi/MidiFile.h>
#include <iostream>
#include <iomanip>

#include <args.h>
#include <cstdlib>
#include <iostream>

int main(int const argc, char const *const *const argv)
{
    // Argument Parsing
    args::ArgumentParser parser("Plays a Suit MIDI Choreo.");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::CompletionFlag completion(parser, {"complete"});
    args::Positional<std::string> midi_filename_arg(parser, "midi_file", "midi file", args::Options::Required);

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

    // Parse MIDI file into sequence of serial messages
    smf::MidiFile midifile;
    midifile.read(midi_filename);
//    midifile.doTimeAnalysis();
//    midifile.linkNotePairs();

    int tracks = midifile.getTrackCount();
    std::cout << "TPQ: " << midifile.getTicksPerQuarterNote() << '\n';
    if (tracks >= 1) std::cout << "TRACKS: " << tracks << '\n';
    for (int track = 0; track < tracks; track++)
    {
        if (tracks > 1) std::cout << "\nTrack " << track << '\n';
        std::cout << "Tick\tSeconds\tDur\tMessage" << '\n';
        std::cout << midifile[track].size() << '\n';
        for (int event = 0; event < midifile[track].size(); event++)
        {
            std::cout << std::dec << midifile[track][event].tick;
            std::cout << '\t' << std::dec << midifile[track][event].seconds;
            std::cout << '\t';
            if (midifile[track][event].isNoteOn())
                std::cout << midifile[track][event].getDurationInSeconds();
            std::cout << '\t' << std::hex;
            for (int i = 0; i < midifile[track][event].size(); i++)
                std::cout << (int) midifile[track][event][i] << ' ';
            std::cout << '\n';
        }
    }

    // Wait for the user to start the chore, or until a midi message is received?

    // Transmit messages
    return EXIT_SUCCESS;
}