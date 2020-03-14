import pathlib
from bitarray import bitarray
import argparse
import mido

middle_c = 60


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

    num_suits = 6
    num_channels = 8

    # A binary string representing the state

    choreo_by_suit = [[] for _ in range(num_suits)]
    states = [bitarray(num_channels) for _ in range(num_suits)]
    for state in states:
        state.setall(0)

    # we use centiseconds for time, 100 centiseconds per second.
    float_time_in_centiseconds = 0.0
    for midi_event in midifile:
        if midi_event.type == 'note_on' or midi_event.type == 'note_off':
            note = midi_event.note - middle_c
            float_time_in_centiseconds += midi_event.time * 100
            suit_number = int(note // num_channels)
            channel = note % num_channels
            bit_idx = num_channels - channel - 1
            if midi_event.type == 'note_on':
                states[suit_number][bit_idx] = True
            elif midi_event.type == 'note_off':
                states[suit_number][bit_idx] = False

            # check if the state has actually changed. if it hasn't dont add the event
            new_state = states[suit_number]
            int_time_in_centiseconds = int(round(float_time_in_centiseconds, 0))
            if len(choreo_by_suit[suit_number]) > 0:
                last_time, last_state = choreo_by_suit[suit_number][-1]
                if last_state == new_state:
                    continue
                elif last_time == int_time_in_centiseconds:
                    # edit the previous event
                    choreo_by_suit[suit_number][-1] = (int_time_in_centiseconds, new_state.copy())
                    states[suit_number] = new_state.copy()
                    continue

            new_event = (int_time_in_centiseconds, states[suit_number].copy())
            choreo_by_suit[suit_number].append(new_event)

    for t, s in choreo_by_suit[0]:
        print(t, s)

    # convert into one long byte string
    bytes_strings = []
    for suit_number in range(num_suits):
        byte_str = bytes()
        num_events = len(choreo_by_suit[suit_number])
        for time_cs, state in choreo_by_suit[suit_number]:
            time_bytes = time_cs.to_bytes(2, 'little')
            state_byte = state.tobytes()
            byte_str += time_bytes + state_byte
        bytes_strings.append(byte_str)

        out_filename = args.outdir / "suit_{}.h".format(suit_number + 1)
        with open(out_filename, 'w') as f:
            f.write("constexpr uint8_t const suit_number = {};\n".format(suit_number + 1))
            f.write("constexpr uint16_t const num_events = {};\n".format(num_events))
            c_str = ", ".join(["0x{:02X}".format(x) for x in list(byte_str)])
            f.write("constexpr uint8_t const choreo[] PROGMEM = {{ {} }};\n".format(c_str))


if __name__ == '__main__':
    main()
