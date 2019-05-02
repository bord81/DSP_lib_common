from lib_bind import *
import matplotlib
import numpy as np
import matplotlib.pyplot as plt
import sys

PI = 3.14159265


def print_usage_and_exit():
    print "Front-end script for DSP_lib_common"
    print "Use as 'python dsp_graphs.py FILE ARG'"
    print "ARGS:"
    print "-hist"
    print "-amp"
    print "-dft"
    sys.exit(1)


def get_data_array_and_size(desc):
    size = get_sound_data_size(desc)
    data = get_sound_data(desc)
    data = cast(data, POINTER(c_double))
    arr = []
    for i in range(size):
        arr.append(data[i])
    return arr, size


def show_hist(desc):
    x, size = get_data_array_and_size(desc)
    num_bins = size / 200
    mean = 0
    stddev = 0
    for i in range(size):
        mean += x[i]
    mean /= size
    for i in range(size):
        stddev += pow(x[i] - mean, 2)
    stddev /= size
    stddev = np.sqrt(stddev)
    fig, ax = plt.subplots()
    n, bins, patches = ax.hist(x, num_bins)
    ax.set_xlabel('Sample value')
    ax.set_ylabel('Number of occurences')
    title = "Histogram of %s, $\mu=%s$, $\sigma=%s$" % (sys.argv[1], mean, stddev)
    ax.set_title(title)
    plt.show()


def rect_to_pol(re, im):
    size = len(re)
    mag = [0] * size
    phase = [0] * size
    for i in range(size):
        mag[i] = np.sqrt(pow(re[i], 2) + pow(im[i], 2))
        if re[i] == 0:
            re[i] = 1e-20
        phase[i] = np.arctan(im[i] / re[i])
    return mag, phase


def pol_to_rect(mag, phase):
    size = len(mag)
    re = [0] * size
    im = [0] * size
    for i in range(size):
        re[i] = mag[i] * np.cos(phase[i])
        im[i] = mag[i] * np.sin(phase[i])
    return re, im


def calc_dft(x):
    size = len(x)
    sizex = (size / 2) + 1
    re = [0] * sizex
    im = [0] * sizex
    for i in range(sizex):
        for k in range(size):
            re[i] += x[k] * np.cos(2 * PI * i * k / size)
            im[i] -= x[k] * np.sin(2 * PI * i * k / size)
    return re, im


def show_dft(desc):
    x, size = get_data_array_and_size(desc)
    re, im = calc_dft(x)
    mag, phase = rect_to_pol(re, im)
    y = np.linspace(0, get_sample_rate(desc) / 2, len(mag))
    t = np.arange(size)
    plt.subplot(2, 1, 1)
    plt.plot(t, x)
    title = "Original signal in %s" % (sys.argv[1])
    plt.title(title)
    xlabel = "DFT of %s" % (sys.argv[1])
    plt.xlabel(xlabel)
    plt.subplot(2, 1, 2)
    plt.plot(y, mag)
    plt.xlabel('Frequency')
    plt.show()


def show_ampl(desc):
    x, size = get_data_array_and_size(desc)
    t = np.arange(size)
    fig, ax = plt.subplots()
    ax.plot(t, x)
    ax.set_xlabel('Time')
    ax.set_ylabel('Amplitude')
    title = "Signal in %s" % (sys.argv[1])
    ax.set_title(title)
    plt.show()


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print_usage_and_exit()
    in_file = open_wav_file(sys.argv[1])
    if sys.argv[2] == "-hist":
        show_hist(in_file)
    elif sys.argv[2] == "-amp":
        show_ampl(in_file)
    elif sys.argv[2] == "-dft":
        show_dft(in_file)
    finish_dsp()
    sys.exit(0)
