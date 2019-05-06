from ctypes import *

cdll.LoadLibrary("libdsp_lib.so")
_dsp_lib = CDLL("libdsp_lib.so")


open_wav_file = _dsp_lib.open_wav_file
open_wav_file.argtypes = (c_char_p,)
open_wav_file.restype = c_int


open_new_file = _dsp_lib.open_new_file
open_new_file.argtypes = (c_char_p,)
open_new_file.restype = c_int


write_file_to_wav = _dsp_lib.write_file_to_wav
write_file_to_wav.argtypes = (c_int,)
write_file_to_wav.restype = c_int

get_sound_data = _dsp_lib.get_sound_data
get_sound_data.argtypes = (c_int,)
get_sound_data.restype = POINTER(c_double)


get_sound_data_size = _dsp_lib.get_sound_data_size
get_sound_data_size.argtypes = (c_int,)
get_sound_data_size.restype = c_longlong


get_sample_rate = _dsp_lib.get_sample_rate
get_sample_rate.argtypes = (c_int,)
get_sample_rate.restype = c_int


amplify = _dsp_lib.amplify
amplify.argtypes = (c_double, c_int, c_int,)
amplify.restype = c_void_p


attenuate = _dsp_lib.attenuate
attenuate.argtypes = (c_double, c_int, c_int,)
attenuate.restype = c_void_p


low_pass = _dsp_lib.low_pass
low_pass.argtypes = (c_int, c_int, c_int, c_int,)
low_pass.restype = c_longlong


high_pass = _dsp_lib.high_pass
high_pass.argtypes = (c_int, c_int, c_int, c_int,)
high_pass.restype = c_longlong


band_pass = _dsp_lib.band_pass
band_pass.argtypes = (c_int, c_int, c_int, c_int, c_int,)
band_pass.restype = c_longlong


band_stop = _dsp_lib.band_stop
band_stop.argtypes = (c_int, c_int, c_int, c_int, c_int,)
band_stop.restype = c_longlong


echo = _dsp_lib.echo
echo.argtypes = (c_int, c_double, c_int, c_int,)
echo.restype = c_void_p


gaussian = _dsp_lib.gaussian
gaussian.argtypes = (c_double, c_int, c_int,)
gaussian.restype = c_void_p


def finish_dsp():
    _dsp_lib.finish_algos()
