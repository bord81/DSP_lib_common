#ifndef DSP_LIB_DSP_ALGOS_H
#define DSP_LIB_DSP_ALGOS_H

// Sample rates for .dat files from BBB dev board. These are set by the AD7172 hardware.
// Consult the AD7172 datasheet for more info.
#define SAMP_RATE_31250 5
#define SAMP_RATE_15625 6
#define SAMP_RATE_10417 7
#define SAMP_RATE_5208  8
#define SAMP_RATE_2604  9
#define SAMP_RATE_1008  10
#define SAMP_RATE_504   11
#define SAMP_RATE_400P6 12
#define SAMP_RATE_200P3 13
#define SAMP_RATE_100P2 14
#define SAMP_RATE_59P98 15
#define SAMP_RATE_50    16

// Must be called after using library to perform cleanup
extern "C" void finish_algos();

// Returns descriptor used later in 'data_*' arguments, accepts filename to open
// Opens only existing files, returning -1 on error
extern "C" int open_wav_file(const char *file_name);

// Creates a new file descriptor used later in 'data_out' arguments, accepts filename to create
// Can't fail as the actual file is not created at this point
extern "C" int open_new_file(const char *file_name);

// Writes file with a given descriptor to a PCM WAV format file on disk
// Returns -1 on error
extern "C" int write_file_to_wav(int desc);

// Returns pointer to sound data using a given descriptor, use with get_sound_data_size!
// Returns nullptr if descriptor is invalid
extern "C" double *get_sound_data(int desc);

// Returns size of sound data, used with get_sound_data
// Returns 0 if descriptor is invalid
extern "C" unsigned long long get_sound_data_size(int desc);

// Returns sample rate of sound data
// Returns 0 if descriptor is invalid
extern "C" int get_sample_rate(int desc);

// Amplifies 'data_in' by 'strength' ranging from 1 to 10
extern "C" void amplify(double strength, int data_in, int data_out);

// Attenuates 'data_in' by 'strength' ranging from 1 to 10
extern "C" void attenuate(double strength, int data_in, int data_out);

// Applies low-pass filter to 'data_in' pushing result to 'data_out'
// 'cutoff_freq' is in Hz and must be less than Nyquist frequency
// 'w_func' can be: 1 - Blackman, 2 - Hanning, 3 - Hamming
// Returns -1 on error
extern "C" long long low_pass(int cutoff_freq, int w_func, int data_in, int data_out);

// Applies high-pass filter to 'data_in' pushing result to 'data_out'
// 'cutoff_freq' is in Hz and must be less than Nyquist frequency
// 'w_func' can be: 1 - Blackman, 2 - Hanning, 3 - Hamming
// Returns -1 on error
extern "C" long long high_pass(int cutoff_freq, int w_func, int data_in, int data_out);

// Applies band-pass filter to 'data_in' pushing result to 'data_out'
// 'cutoff_lo' and 'cutoff_hi' are in Hz and must be less than Nyquist frequency
// 'w_func' can be: 1 - Blackman, 2 - Hanning, 3 - Hamming
// Returns -1 on error
extern "C" long long
band_pass(int cutoff_lo, int cutoff_hi, int w_func, int data_in, int data_out);

// Applies band-stop filter to 'data_in' pushing result to 'data_out'
// 'cutoff_lo' and 'cutoff_hi' are in Hz and must be less than Nyquist frequency
// 'w_func' can be: 1 - Blackman, 2 - Hanning, 3 - Hamming
// Returns -1 on error
extern "C" long long
band_stop(int cutoff_lo, int cutoff_hi, int w_func, int data_in, int data_out);

// Applies simple echo to 'data_in' pushing result to 'data_out'
// 'delay' is the coefficient applied to sample rate to get delay
extern "C" void echo(int delay, double coef, int data_in, int data_out);


#endif //DSP_LIB_DSP_ALGOS_H
