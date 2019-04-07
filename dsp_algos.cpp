#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <mutex>
#include <climits>
#include <chrono>

#include "dsp_algos.h"
#include "wav_h_gen.h"
#include "wsfir.h"

static_assert(sizeof(float) * CHAR_BIT == 32, "require 32 bits floats");

using namespace std;
namespace dsp_algos {

    struct Data {
        string file_path;
        size_t size;
        float *samples;
        int sample_rate;
        bool allocated;
    };

    struct WindowFilter {
        double *taps;
        int size;
        bool allocated;
    };


    static vector<Data> records;
    static vector<WindowFilter> w_filters;
    static mutex data_guard;

    static const auto FILTER_SIZE = 100;

    static bool isValid(int desc) {
        return (desc < records.size()) && records[desc].allocated;
    }

    static size_t getDataSize(int desc) {
        return records[desc].size;
    }

    static float *getSamples(int desc) {
        return records[desc].samples;
    }

    //returns real sample rate for calculations
    //open/write procedures must use sample_rate field from struct instead
    static int getSampleRate_int(int desc) {
        switch (records[desc].sample_rate) {
            case SAMP_RATE_31250 :
            default:
                return 31250;
            case SAMP_RATE_15625 :
                return 15625;
            case SAMP_RATE_10417 :
                return 10417;
            case SAMP_RATE_5208 :
                return 5208;
            case SAMP_RATE_2604 :
                return 2604;
            case SAMP_RATE_1008 :
                return 1008;
            case SAMP_RATE_504 :
                return 504;
            case SAMP_RATE_400P6 :
                return 381;
            case SAMP_RATE_200P3 :
                return 200;
            case SAMP_RATE_100P2 :
                return 100;
            case SAMP_RATE_59P98 :
                return 59;
            case SAMP_RATE_50 :
                return 50;
        }
    }

    static bool isNyqFreq(int desc, int freq) {
        return freq < getSampleRate_int(desc) / 2;
    }

    static bool isWFuncValid(int w_func) {
        return w_func > 0 && w_func < 4;
    }

    static bool allocMem(int desc, size_t size) {
        if (!records[desc].allocated) {
            records[desc].samples = static_cast<float *>(calloc(size, sizeof(float)));
            records[desc].allocated = true;
            records[desc].size = size;
            return records[desc].samples != nullptr;
        } else {
            return false;
        }
    }

    static int makeWFilter(function<void(double *)> f) {
        WindowFilter w_f;
        w_f.size = 0;
        w_f.allocated = false;
        w_f.taps = nullptr;
        w_filters.push_back(w_f);
        Data stub;
        stub.sample_rate = 0;
        stub.allocated = false;
        stub.file_path = "__stub";
        stub.samples = nullptr;
        stub.size = 0;
        records.push_back(stub);
        records[records.size() - 1].samples = static_cast<float *>(calloc(FILTER_SIZE, sizeof(float)));
        w_filters[w_filters.size() - 1].taps = static_cast<double *>(calloc(FILTER_SIZE, sizeof(double)));
        if (records[records.size() - 1].samples != nullptr &&
            w_filters[w_filters.size() - 1].taps != nullptr) {
            records[records.size() - 1].size = FILTER_SIZE;
            w_filters[w_filters.size() - 1].size = FILTER_SIZE;
            records[records.size() - 1].allocated = true;
            f(w_filters[w_filters.size() - 1].taps);
            for (int i = 0; i < w_filters[w_filters.size() - 1].size; ++i) {
                records[records.size() - 1].samples[i] = static_cast<float>(w_filters[w_filters.size() -
                                                                                      1].taps[i]);
            }
        } else {
            return -1;
        }
        return static_cast<int>(records.size() - 1);
    }

    static void cutOverflowRaw(int desc) {
        for (size_t i = 0; i < records[desc].size; ++i) {
            if (records[desc].samples[i] < -10.0f) {
                records[desc].samples[i] = -10.0f;
            } else if (records[desc].samples[i] > 10.0f) {
                records[desc].samples[i] = 10.0f;
            }
        }
    }

    extern "C" void finish_algos() {
        lock_guard<mutex> lock(data_guard);
        for_each(records.begin(), records.end(), [](Data &data) {
            if (data.allocated) {
                free(data.samples);
                data.allocated = false;
            }
        });
        for_each(w_filters.begin(), w_filters.end(), [](WindowFilter &wf) {
            if (wf.allocated) {
                free(wf.taps);
                wf.allocated = false;
            }
        });
    }

    extern "C" int open_dat_file(const char *file_name) {
        lock_guard<mutex> lock(data_guard);
        FILE *data_file;
        data_file = fopen(file_name, "r");
        if (data_file != nullptr) {
            Data data;
            data.allocated = false;
            data.file_path = file_name;
            data.samples = nullptr;
            int vs_24 = fgetc(data_file);
            int vs_16 = fgetc(data_file);
            int vs_8 = fgetc(data_file);
            int vs_lo = fgetc(data_file);
            data.sample_rate = fgetc(data_file);
            int size = vs_24 << 24;
            size |= vs_16 << 16;
            size |= vs_8 << 8;
            size |= vs_lo;
            data.size = size;
            records.push_back(data);
            if (allocMem((records.size() - 1), size)) {
                fread(records[records.size() - 1].samples, sizeof(float),
                      static_cast<size_t>(size), data_file);
            } else {
                fclose(data_file);
                return -1;
            }
            fclose(data_file);
        } else {
            return -1;
        }
        return static_cast<int>(records.size() - 1);
    }

    extern "C" int open_new_file(const char *file_name) {
        lock_guard<mutex> lock(data_guard);
        Data data;
        data.sample_rate = 0;
        data.allocated = false;
        data.file_path = file_name;
        data.samples = nullptr;
        data.size = 0;
        records.push_back(data);
        return static_cast<int>(records.size() - 1);
    }

    extern "C" int write_file_to_dat(int desc) {
        lock_guard<mutex> lock(data_guard);
        if (isValid(desc)) {
            uint8_t vs_24 = static_cast<uint8_t>(getDataSize(desc) >> 24);
            uint8_t vs_16 = static_cast<uint8_t>(getDataSize(desc) >> 16);
            uint8_t vs_8 = static_cast<uint8_t>(getDataSize(desc) >> 8);
            uint8_t vs_lo = static_cast<uint8_t>(getDataSize(desc));
            uint8_t sr = static_cast<uint8_t>(records[desc].sample_rate);
            FILE *data_file;
            data_file = fopen(records[desc].file_path.c_str(), "w");
            if (data_file == nullptr) {
                perror("Following error occured: ");
            } else {
                fputc(vs_24, data_file);
                fputc(vs_16, data_file);
                fputc(vs_8, data_file);
                fputc(vs_lo, data_file);
                fputc(sr, data_file);
                fwrite(getSamples(desc), sizeof(float),
                       static_cast<size_t>(getDataSize(desc)), data_file);
                fflush(data_file);
                fclose(data_file);
            }
        } else {
            return -1;
        }
        return 0;
    }

    extern "C" int write_file_to_wav(int desc) {
        lock_guard<mutex> lock(data_guard);
        if (isValid(desc)) {
            FILE *wav_file;
            wav_h_gen h_gen;
            int16_t *out_volts = static_cast<int16_t *>(calloc(getDataSize(desc), sizeof(int16_t)));
            for (int i = 0; i < getDataSize(desc); ++i) {
                out_volts[i] = static_cast<int16_t>(getSamples(desc)[i] * 3276.7);
            }
            wav_file = fopen(records[desc].file_path.c_str(), "w");
            if (wav_file == nullptr) {
                perror("Following error occured: ");
            } else {
                wav_header *header = h_gen.get_wav_header(getDataSize(desc), getSampleRate_int(desc));
                fwrite(header->chunk_id, 4, 1, wav_file);
                fwrite(&header->chunk_size, 4, 1, wav_file);
                fwrite(header->chunk_format, 4, 1, wav_file);
                fwrite(header->sub_chunk_1_id, 4, 1, wav_file);
                fwrite(header->sub_chunk_1_size, 4, 1, wav_file);
                fwrite(header->audio_format, 2, 1, wav_file);
                fwrite(header->num_channels, 2, 1, wav_file);
                fwrite(&header->sample_rate, 4, 1, wav_file);
                fwrite(&header->byte_rate, 4, 1, wav_file);
                fwrite(header->block_align, 2, 1, wav_file);
                fwrite(header->bits_sample, 2, 1, wav_file);
                fwrite(header->sub_chunk_2_id, 4, 1, wav_file);
                fwrite(&header->sub_chunk_2_size, 4, 1, wav_file);
                int res = fwrite(out_volts, sizeof(int16_t), static_cast<size_t>(getDataSize(desc)), wav_file);
                fflush(wav_file);
                fclose(wav_file);
            }
            free(out_volts);
        } else {
            return -1;
        }
        return 0;
    }

    extern "C" float *get_sound_data(int desc) {
        lock_guard<mutex> lock(data_guard);
        if (isValid(desc)) {
            return getSamples(desc);
        }
        return nullptr;
    }

    extern "C" unsigned long long get_sound_data_size(int desc) {
        lock_guard<mutex> lock(data_guard);
        if (isValid(desc)) {
            return getDataSize(desc);
        }
        return 0LL;
    }

    extern "C" int get_sample_rate(int desc) {
        lock_guard<mutex> lock(data_guard);
        if (isValid(desc)) {
            return getSampleRate_int(desc);
        }
        return 0;
    }

    extern "C" long long convolve(int data_in, int data_resp, int data_out) {
        lock_guard<mutex> lock(data_guard);
        if (!isValid(data_in) ||
            !isValid(data_resp) ||
            !allocMem(data_out, getDataSize(data_in) + getDataSize(data_resp) - 1)) {
            return -1;
        }
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
        for (size_t i = 0; i < getDataSize(data_in); ++i) {
            for (size_t j = 0; j < getDataSize(data_resp); ++j) {
                getSamples(data_out)[i + j] = getSamples(data_out)[i + j]
                                              + getSamples(data_in)[i]
                                                * getSamples(data_resp)[j];
            }
        }
        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        return (t2 - t1).count();
    }

    extern "C" void amplify(int strength, int data_in, int data_out) {
        lock_guard<mutex> lock(data_guard);
        if (isValid(data_in) && allocMem(data_out, getDataSize(data_in))) {
            float mult;
            if (strength == 0) {
                mult = 2.0;
            } else {
                mult = strength / 10.0 + 1.0;
            }
            for (int i = 0; i < getDataSize(data_in); ++i) {
                getSamples(data_out)[i] = getSamples(data_in)[i] * mult;
            }
            cutOverflowRaw(data_out);
        }
    }

    extern "C" void attenuate(int strength, int data_in, int data_out) {
        lock_guard<mutex> lock(data_guard);
        if (isValid(data_in) && allocMem(data_out, getDataSize(data_in))) {
            float mult;
            if (strength == 0) {
                mult = 0.5;
            } else {
                mult = 1.0 - strength / 20.0;
            }
            for (int i = 0; i < getDataSize(data_in); ++i) {
                getSamples(data_out)[i] = getSamples(data_in)[i] * mult;
            }
            cutOverflowRaw(data_out);
        }
    }

    extern "C" long long low_pass(int cutoff_freq, int w_func, int data_in, int data_out) {
        lock_guard<mutex> lock(data_guard);
        const double cf = static_cast<double >(cutoff_freq) / getSampleRate_int(data_in);
        if (isValid(data_in) &&
            isNyqFreq(data_in, cutoff_freq) &&
            isWFuncValid(w_func)) {
            int data_filter = makeWFilter([w_func, cf](double *fd) {
                wsfirLP(fd, FILTER_SIZE, w_func, cf);
            });
            return data_filter != -1 ? convolve(data_in, data_filter, data_out) : -1;
        }
        return -1;
    }

    extern "C" long long high_pass(int cutoff_freq, int w_func, int data_in, int data_out) {
        lock_guard<mutex> lock(data_guard);
        const double cf = static_cast<double >(cutoff_freq) / getSampleRate_int(data_in);
        if (isValid(data_in) &&
            isNyqFreq(data_in, cutoff_freq) &&
            isWFuncValid(w_func)) {
            int data_filter = makeWFilter([w_func, cf](double *fd) {
                wsfirHP(fd, FILTER_SIZE, w_func, cf);
            });
            return data_filter != -1 ? convolve(data_in, data_filter, data_out) : -1;
        }
        return -1;
    }

    extern "C" long long band_pass(int cutoff_lo, int cutoff_hi, int w_func, int data_in, int data_out) {
        lock_guard<mutex> lock(data_guard);
        const double cf_lo = static_cast<double >(cutoff_lo) / getSampleRate_int(data_in);
        const double cf_hi = static_cast<double >(cutoff_hi) / getSampleRate_int(data_in);
        if (isValid(data_in) &&
            isNyqFreq(data_in, cutoff_lo) &&
            isNyqFreq(data_in, cutoff_hi) &&
            cutoff_lo < cutoff_hi &&
            isWFuncValid(w_func)) {
            int data_filter = makeWFilter([w_func, cf_lo, cf_hi](double *fd) {
                wsfirBP(fd, FILTER_SIZE, w_func, cf_lo, cf_hi);
            });
            return data_filter != -1 ? convolve(data_in, data_filter, data_out) : -1;
        }
        return -1;
    }

    extern "C" long long band_stop(int cutoff_lo, int cutoff_hi, int w_func, int data_in, int data_out) {
        lock_guard<mutex> lock(data_guard);
        const double cf_lo = static_cast<double >(cutoff_lo) / getSampleRate_int(data_in);
        const double cf_hi = static_cast<double >(cutoff_hi) / getSampleRate_int(data_in);
        if (isValid(data_in) &&
            isNyqFreq(data_in, cutoff_lo) &&
            isNyqFreq(data_in, cutoff_hi) &&
            cutoff_lo < cutoff_hi &&
            isWFuncValid(w_func)) {
            int data_filter = makeWFilter([w_func, cf_lo, cf_hi](double *fd) {
                wsfirBS(fd, FILTER_SIZE, w_func, cf_lo, cf_hi);
            });
            return data_filter != -1 ? convolve(data_in, data_filter, data_out) : -1;
        }
        return -1;
    }

    extern "C" void echo(int delay, float coef, int data_in, int data_out) {
        lock_guard<mutex> lock(data_guard);
        if (isValid(data_in) && allocMem(data_out, getDataSize(data_in) + delay)) {
            for (int i = 0; i < getDataSize(data_in); ++i) {
                getSamples(data_out)[i] += getSamples(data_in)[i];
                getSamples(data_out)[i + delay] += getSamples(data_in)[i] * coef;
            }
        }
    }
}
