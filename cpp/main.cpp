#include <args.h>
#include <cstdlib>
#include <iostream>

int main(int const argc, char const *const *const argv)
{
    // Argument Parsing
    args::ArgumentParser parser("Plays a Suit MIDI Choreo.");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::CompletionFlag completion(parser, {"complete"});
    args::Positional<std::string> midi_filename_arg(parser, "midi_file", "midi file");

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
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    auto const midi_filename = midi_filename_arg.Get();

    // Parse MIDI file into sequence of serial messages

    // Wait for the user to start the chore, or until a midi message is received?

    // Transmit messages
    return EXIT_SUCCESS;
}