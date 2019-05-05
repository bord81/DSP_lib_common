from lib_bind import *
import sys


def print_usage_and_exit():
    print "Filters script for DSP_lib_common"
    print "Use as 'python dsp_filters.py FILE_IN FILE_OUT ARG KEYS'"
    print "ARGS/KEYS:"
    print "-amp %strength"
    print "-att %strength"
    print "-lp %cutoff-frequency %window-filter"
    print "-hp %cutoff-frequency %window-filter"
    print "-bp %lower-frequency %upper-frequency %window-filter"
    print "-bs %lower-frequency %upper-frequency %window-filter"
    print "-ec %delay %coef"
    print "-gau %coef"
    sys.exit(1)


if __name__ == "__main__":
    if len(sys.argv) < 5:
        print_usage_and_exit()
    in_file = open_wav_file(sys.argv[1])
    out_file = open_new_file(sys.argv[2])
    print in_file
    print out_file
    if len(sys.argv) == 5:
        if sys.argv[3] == "-amp":
            amplify(float(sys.argv[4]), in_file, out_file)
        elif sys.argv[3] == "-att":
            attenuate(float(sys.argv[4]), in_file, out_file)
        elif sys.argv[3] == "-gau":
            gaussian(float(sys.argv[4]), in_file, out_file)
    elif len(sys.argv) == 6:
        print "Filter action took:"
        if sys.argv[3] == "-lp":
            print low_pass(int(sys.argv[4]), int(sys.argv[5]), in_file, out_file)
        elif sys.argv[3] == "-hp":
            print high_pass(int(sys.argv[4]), int(sys.argv[5]), in_file, out_file)
        elif sys.argv[3] == "-ec":
            echo(int(sys.argv[4]), float(sys.argv[5]), in_file, out_file)
    elif len(sys.argv) == 7:
        print "Filter action took:"
        if sys.argv[3] == "-bp":
            print band_pass(int(sys.argv[4]), int(sys.argv[5]), int(sys.argv[6]), in_file, out_file)
        elif sys.argv[3] == "-bs":
            print band_stop(int(sys.argv[4]), int(sys.argv[5]), int(sys.argv[6]), in_file, out_file)
    print write_file_to_wav(out_file)
    finish_dsp()
    print "Done!"
    sys.exit(0)
