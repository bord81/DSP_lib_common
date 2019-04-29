from ctypes import *

cdll.LoadLibrary("libdsp_lib.so")
_dsp_lib = CDLL("libdsp_lib.so")

def open_wav_file(file_name):
    return _dsp_lib.open_wav_file(c_char_p(file_name))

def open_new_file(file_name):
    return _dsp_lib.open_new_file(c_char_p(file_name))

def write_file_to_wav(desc):
    return _dsp_lib.write_file_to_wav(desc)

def get_sound_data(desc):
    return _dsp_lib.get_sound_data(desc)

def get_sound_data_size(desc):
    return _dsp_lib.get_sound_data_size(desc)

def get_sample_rate(desc):
    return _dsp_lib.get_sample_rate(desc)

def amplify(strength, data_in, data_out):
    _dsp_lib.amplify(c_double(strength), data_in, data_out)

def attenuate(strength, data_in, data_out):
    _dsp_lib.attenuate(c_double(strength), data_in, data_out)

def low_pass(cutoff_freq, w_func, data_in, data_out):
    return _dsp_lib.low_pass(cutoff_freq, w_func, data_in, data_out)

def high_pass(cutoff_freq, w_func, data_in, data_out):
    return _dsp_lib.high_pass(cutoff_freq, w_func, data_in, data_out)

def band_pass(cutoff_lo, cutoff_hi, w_func, data_in, data_out):
    return _dsp_lib.band_pass(cutoff_lo, cutoff_hi, w_func, data_in, data_out)

def band_stop(cutoff_lo, cutoff_hi, w_func, data_in, data_out):
    return _dsp_lib.band_stop(cutoff_lo, cutoff_hi, w_func, data_in, data_out)

def echo(delay, coef, data_in, data_out):
    return _dsp_lib.echo(delay, c_double(coef), data_in, data_out)

def finish_dsp():
    _dsp_lib.finish_algos()
