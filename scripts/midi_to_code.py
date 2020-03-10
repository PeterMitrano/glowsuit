import pathlib
from bitarray import bitarray
import argparse
import mido


def delta_ticks_to_seconds(beats_per_minute, ticks_per_beat, delta_ticks):
    seconds_per_minute = 60.0
    seconds_per_beat = seconds_per_minute / beats_per_minute
    seconds_per_tick = seconds_per_beat / float(ticks_per_beat)
    time_in_seconds = delta_ticks * seconds_per_tick
    return time_in_seconds


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("midi_file", type=pathlib.Path)
    parser.add_argument("outdir", type=pathlib.Path)

    args = parser.parse_args()

    midifile = mido.MidiFile(args.midi_file)

    num_suits = 5
    num_channels = 8

    # A binary string representing the state

    choreo_by_suit = [[] for _ in range(num_suits)]
    states = [bitarray(num_channels)] * num_suits
    for state in states:
        state.setall(0)

    # we use centiseconds for time, 100 centiseconds per second.
    time_in_centiseconds = 0
    for midi_event in midifile:
        if midi_event.type == 'note_on' or midi_event.type == 'note_off':
            note = midi_event.note - 64
            time_in_centiseconds += int(midi_event.time * 100)
            suit_number = int(note // num_channels)
            channel = note % num_channels
            bit_idx = num_channels - channel - 1
            if midi_event.type == 'note_on':
                states[suit_number][bit_idx] = True
            elif midi_event.type == 'note_off':
                states[suit_number][bit_idx] = False
            time_in_centiseconds += int(midi_event.time * 100)
            my_event = (time_in_centiseconds, states[suit_number].copy())
            choreo_by_suit[suit_number].append(my_event)

    # convert into one long byte string
    bytes_strings = []
    for suit_number in range(num_suits):
        byte_str = bytes()
        for time_cs, state in choreo_by_suit[suit_number]:
            time_bytes = time_cs.to_bytes(2, 'big')
            state_byte = state.tobytes()
            byte_str += time_bytes + state_byte
        bytes_strings.append(byte_str)

        out_filename = args.outdir / "suit_{}.h".format(suit_number + 1)
        size = len(byte_str)
        with open(out_filename, 'w') as f:
            f.write("constexpr unsigned int const suit_number = {};\n".format(suit_number + 1))
            f.write("constexpr unsigned int const size = {};\n".format(size))
            c_str = ", ".join(["0x{:02X}".format(x) for x in list(byte_str)])
            f.write("constexpr uint8_t const choreo[] PROGMEM = {{ {} }};\n".format(c_str))


if __name__ == '__main__':
    main()
