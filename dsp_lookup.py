from lib_bind import *
import os, sys

def print_usage_and_exit():
    print "Front-end script for DSP_lib_common"
    print "Use as 'python dsp_lookup.py FILE ARG'"
    print "ARGS:"
    print "-sr"
    print "-sz"
    sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print_usage_and_exit()
    in_file = open_wav_file(sys.argv[1])
    if sys.argv[2] == "-sr":
        print "Sample rate:"
        print get_sample_rate(in_file)
    elif sys.argv[2] == "-sz":
        print "Samples count:"
        print get_sound_data_size(in_file)
    finish_dsp()
    sys.exit(0)
