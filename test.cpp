#include <dlfcn.h>
#include <cstdio>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Program must be run with test file path as the only argument.\n");
        return 1;
    }
    typedef int open_dat_file_t(const char *file_name);
    typedef int open_new_file_t(const char *file_name);
    typedef int write_file_to_wav_t(int desc);
    typedef unsigned long long get_sound_data_size_t(int desc);
    typedef int get_sample_rate_t(int desc);
    typedef void amplify_t(int strength, int data_in, int data_out);
    typedef void attenuate_t(int strength, int data_in, int data_out);
    typedef long long low_pass_t(int cutoff_freq, int w_func, int data_in, int data_out);
    typedef long long high_pass_t(int cutoff_freq, int w_func, int data_in, int data_out);
    typedef long long band_pass_t(int cutoff_lo, int cutoff_hi, int w_func, int data_in, int data_out);
    typedef long long band_stop_t(int cutoff_lo, int cutoff_hi, int w_func, int data_in, int data_out);
    typedef void echo_t(int delay, float coef, int data_in, int data_out);
    typedef void finish_algos_t();

    void *handle = dlopen("./libdsp_lib.so", RTLD_LAZY);
    if (handle == nullptr) {
        printf("%s\n", dlerror());
        return 1;
    }
    auto open_dat_file = (open_dat_file_t *) dlsym(handle, "open_dat_file");
    auto open_new_file = (open_new_file_t *) dlsym(handle, "open_new_file");
    auto write_file_to_wav = (write_file_to_wav_t *) dlsym(handle, "write_file_to_wav");
    auto get_sound_data_size = (get_sound_data_size_t *) dlsym(handle, "get_sound_data_size");
    auto get_sample_rate = (get_sample_rate_t *) dlsym(handle, "get_sample_rate");
    auto amplify = (amplify_t *) dlsym(handle, "amplify");
    auto attenuate = (attenuate_t *) dlsym(handle, "attenuate");
    auto low_pass = (low_pass_t *) dlsym(handle, "low_pass");
    auto high_pass = (high_pass_t *) dlsym(handle, "high_pass");
    auto band_pass = (band_pass_t *) dlsym(handle, "band_pass");
    auto band_stop = (band_stop_t *) dlsym(handle, "band_stop");
    auto echo = (echo_t *) dlsym(handle, "echo");
    auto finish_algos = (finish_algos_t *) dlsym(handle, "finish_algos");

    const char *att = "attenuate_test.wav";
    const char *amp = "amplify_test.wav";
    const char *lp = "low_pass_test.wav";
    const char *hp = "high_pass_test.wav";
    const char *bp = "band_pass_test.wav";
    const char *bs = "band_stop_test.wav";
    const char *echo_f = "echo_test.wav";

    printf("Opening test file %s\n", argv[1]);
    int test_file = open_dat_file(argv[1]);
    if (test_file == -1) {
        printf("Couldn't open %s\n", argv[1]);
        return 1;
    }
    int sample_rate = get_sample_rate(test_file);
    int lp_freq = sample_rate / 2 / 15;
    int hp_freq = sample_rate / 2 / 5 * 4;
    int b_freq_lo = sample_rate / 2 / 15;
    int b_freq_hi = sample_rate / 2 / 5 * 5 - 10;
    printf("Sample rate is %d\n", sample_rate);
    int new_file_att = open_new_file(att);
    int new_file_amp = open_new_file(amp);
    int new_file_lp = open_new_file(lp);
    int new_file_hp = open_new_file(hp);
    int new_file_bp = open_new_file(bp);
    int new_file_bs = open_new_file(bs);
    int new_file_echo = open_new_file(echo_f);
    attenuate(10, test_file, new_file_att);
    amplify(10, test_file, new_file_amp);
    long long lp_time = low_pass(lp_freq, 1, test_file, new_file_lp);
    printf("Low pass filter took: %lld\n", lp_time);
    long long hp_time = high_pass(hp_freq, 1, test_file, new_file_hp);
    printf("High pass filter took: %lld\n", hp_time);
    long long bp_time = band_pass(b_freq_lo, b_freq_hi, 1, test_file, new_file_bp);
    printf("Band pass filter took: %lld\n", bp_time);
    long long bs_time = band_stop(b_freq_lo, b_freq_hi, 3, test_file, new_file_bs);
    printf("Band stop filter took: %lld\n", bs_time);
    echo(sample_rate / 2, 0.5, test_file, new_file_echo);
    long long data_size = get_sound_data_size(test_file);
    printf("Sound data size:  %lld\n", data_size);

    write_file_to_wav(new_file_att);
    write_file_to_wav(new_file_amp);
    write_file_to_wav(new_file_lp);
    write_file_to_wav(new_file_bp);
    write_file_to_wav(new_file_hp);
    write_file_to_wav(new_file_bs);
    write_file_to_wav(new_file_echo);

    finish_algos();
    return 0;
}