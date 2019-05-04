#include <cstring>
#include <vector>
#include <algorithm>
#include <functional>
#include <mutex>
#include <climits>
#include <chrono>

extern "C" {
#include <sndfile.h>
}

#include "dsp_algos.h"
#include "wsfir.h"

using namespace std;
namespace dsp_algos {

    struct Data {
        SF_INFO info;
        string file_path;
        double *samples;
        bool allocated;
    };

    static vector<Data> records;
    static mutex data_guard;

    static const auto FILTER_SIZE = 100;
    static const auto BUFFER_LEN = 2048;

    static bool isValid(int desc) {
        return (desc < records.size()) && records[desc].allocated;
    }

    static long long getDataSize(int desc) {
        return records[desc].info.frames * records[desc].info.channels;
    }

    static double *getSamples(int desc) {
        return records[desc].samples;
    }

    static int getSampleRate(int desc) {
        return records[desc].info.samplerate;
    }

    static string& getFileName(int desc) {
        return records[desc].file_path;
    }

    static SF_INFO& getSndInfo(int desc) {
        return records[desc].info;
    }

    static bool isNyqFreq(int desc, int freq) {
        return freq < records[desc].info.samplerate / 2;
    }

    static bool isWFuncValid(int w_func) {
        return w_func > 0 && w_func < 4;
    }

    static void copySndInfo(int in, int out) {
        memcpy(&records[out].info, &records[in].info, sizeof(SF_INFO));
    }

    static bool allocMem(int desc, long long size) {
        if (!records[desc].allocated) {
            records[desc].samples = static_cast<double *>(calloc(size, sizeof(double)));
            records[desc].allocated = true;
            return records[desc].samples != nullptr;
        } else {
            return false;
        }
    }

    static int makeWFilter(const function<void(double *)> &f) {
        Data stub;
        stub.allocated = false;
        stub.file_path = "__stub";
        stub.samples = nullptr;
        memset(&stub.info, 0, sizeof(stub.info));
        stub.info.channels = 1;
        stub.info.frames = FILTER_SIZE;
        records.push_back(stub);
        records[records.size() - 1].samples = static_cast<double *>(calloc(FILTER_SIZE, sizeof(double)));
        if (records[records.size() - 1].samples != nullptr) {
            records[records.size() - 1].allocated = true;
            f(records[records.size() - 1].samples);
        } else {
            return -1;
        }
        return static_cast<int>(records.size() - 1);
    }

    static int makeBFilter(int size) {
        Data stub;
        stub.allocated = false;
        stub.file_path = "__stub";
        stub.samples = nullptr;
        memset(&stub.info, 0, sizeof(stub.info));
        stub.info.channels = 1;
        stub.info.frames = size;
        records.push_back(stub);
        records[records.size() - 1].samples = static_cast<double *>(calloc(size, sizeof(double)));
        if (records[records.size() - 1].samples != nullptr) {
            records[records.size() - 1].allocated = true;
            wBlackman(records[records.size() - 1].samples, size);
        } else {
            return -1;
        }
        return static_cast<int>(records.size() - 1);
    }

    extern "C" void finish_algos() {
        lock_guard<mutex> lock(data_guard);
        for_each(records.begin(), records.end(), [](Data &data) {
            if (data.allocated) {
                free(data.samples);
                data.allocated = false;
            }
        });
    }

    extern "C" int open_wav_file(const char *file_name) {
        lock_guard<mutex> lock(data_guard);
        SNDFILE *data_file = nullptr;
        Data data;
        double buffer[BUFFER_LEN];
        long read_count = 0;
        long offset = 0L;
        long long size = 0LL;
        memset(&data.info, 0, sizeof(data.info));
        data.allocated = false;
        data.file_path = file_name;
        data.samples = nullptr;
        records.push_back(data);
        if ((data_file = sf_open(file_name, SFM_READ, &records[records.size() - 1].info)) != nullptr
                && allocMem((records.size() - 1), (size = records[records.size() - 1].info.frames * records[records.size() - 1].info.channels))) {
            while ((read_count = sf_read_double(data_file, buffer, BUFFER_LEN))) {
                memcpy(records[records.size() - 1].samples + offset, buffer, static_cast<size_t>(read_count * sizeof (double)));
                offset += read_count;
            }
            sf_close(data_file);
        } else {
            return -1;
        }
        return static_cast<int>(records.size() - 1);
    }

    extern "C" int open_new_file(const char *file_name) {
        lock_guard<mutex> lock(data_guard);
        Data data;
        memset(&data.info, 0, sizeof(data.info));
        data.allocated = false;
        data.file_path = file_name;
        data.samples = nullptr;
        records.push_back(data);
        return static_cast<int>(records.size() - 1);
    }

    extern "C" int write_file_to_wav(int desc) {
        lock_guard<mutex> lock(data_guard);
        if (isValid(desc)) {
            SNDFILE *wav_file;
            long long size = getDataSize(desc);
            wav_file = sf_open(getFileName(desc).c_str(), SFM_WRITE, &getSndInfo(desc));
            sf_write_double(wav_file, getSamples(desc), size);
            sf_close(wav_file);
        } else {
            return -1;
        }
        return 0;
    }

    extern "C" double *get_sound_data(int desc) {
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
            return getSampleRate(desc);
        }
        return 0;
    }

    long long convolve(int data_in, int data_resp, int data_out) {
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

    extern "C" void amplify(double strength, int data_in, int data_out) {
        lock_guard<mutex> lock(data_guard);
        if (isValid(data_in) && allocMem(data_out, getDataSize(data_in))) {
            for (int i = 0; i < getDataSize(data_in); ++i) {
                getSamples(data_out)[i] = getSamples(data_in)[i] * strength;
            }
            copySndInfo(data_in, data_out);
        }
    }

    extern "C" void attenuate(double strength, int data_in, int data_out) {
        lock_guard<mutex> lock(data_guard);
        if (isValid(data_in) && allocMem(data_out, getDataSize(data_in))) {
            for (int i = 0; i < getDataSize(data_in); ++i) {
                getSamples(data_out)[i] = getSamples(data_in)[i] / strength;
            }
            copySndInfo(data_in, data_out);
        }
    }

    extern "C" long long low_pass(int cutoff_freq, int w_func, int data_in, int data_out) {
        lock_guard<mutex> lock(data_guard);
        const double cf = static_cast<double >(cutoff_freq) / getSampleRate(data_in);
        if (isValid(data_in) &&
            isNyqFreq(data_in, cutoff_freq) &&
            isWFuncValid(w_func)) {

            int data_filter = makeWFilter([w_func, cf](double *fd) {
                wsfirLP(fd, FILTER_SIZE, w_func, cf);
            });
            copySndInfo(data_in, data_out);
            getSndInfo(data_out).frames += FILTER_SIZE;
            return data_filter != -1 ? convolve(data_in, data_filter, data_out) : -1;
        }
        return -1;
    }

    extern "C" long long high_pass(int cutoff_freq, int w_func, int data_in, int data_out) {
        lock_guard<mutex> lock(data_guard);
        const double cf = static_cast<double >(cutoff_freq) / getSampleRate(data_in);
        if (isValid(data_in) &&
            isNyqFreq(data_in, cutoff_freq) &&
            isWFuncValid(w_func)) {
            int data_filter = makeWFilter([w_func, cf](double *fd) {
                wsfirHP(fd, FILTER_SIZE, w_func, cf);
            });
            copySndInfo(data_in, data_out);
            getSndInfo(data_out).frames += FILTER_SIZE;
            return data_filter != -1 ? convolve(data_in, data_filter, data_out) : -1;
        }
        return -1;
    }

    extern "C" long long band_pass(int cutoff_lo, int cutoff_hi, int w_func, int data_in, int data_out) {
        lock_guard<mutex> lock(data_guard);
        const double cf_lo = static_cast<double >(cutoff_lo) / getSampleRate(data_in);
        const double cf_hi = static_cast<double >(cutoff_hi) / getSampleRate(data_in);
        if (isValid(data_in) &&
            isNyqFreq(data_in, cutoff_lo) &&
            isNyqFreq(data_in, cutoff_hi) &&
            cutoff_lo < cutoff_hi &&
            isWFuncValid(w_func)) {
            int data_filter = makeWFilter([w_func, cf_lo, cf_hi](double *fd) {
                wsfirBP(fd, FILTER_SIZE, w_func, cf_lo, cf_hi);
            });
            copySndInfo(data_in, data_out);
            getSndInfo(data_out).frames += FILTER_SIZE;
            return data_filter != -1 ? convolve(data_in, data_filter, data_out) : -1;
        }
        return -1;
    }

    extern "C" long long band_stop(int cutoff_lo, int cutoff_hi, int w_func, int data_in, int data_out) {
        lock_guard<mutex> lock(data_guard);
        const double cf_lo = static_cast<double >(cutoff_lo) / getSampleRate(data_in);
        const double cf_hi = static_cast<double >(cutoff_hi) / getSampleRate(data_in);
        if (isValid(data_in) &&
            isNyqFreq(data_in, cutoff_lo) &&
            isNyqFreq(data_in, cutoff_hi) &&
            cutoff_lo < cutoff_hi &&
            isWFuncValid(w_func)) {
            int data_filter = makeWFilter([w_func, cf_lo, cf_hi](double *fd) {
                wsfirBS(fd, FILTER_SIZE, w_func, cf_lo, cf_hi);
            });
            copySndInfo(data_in, data_out);
            getSndInfo(data_out).frames += FILTER_SIZE;
            return data_filter != -1 ? convolve(data_in, data_filter, data_out) : -1;
        }
        return -1;
    }

    extern "C" void echo(int delay, double coef, int data_in, int data_out) {
        lock_guard<mutex> lock(data_guard);
        if (isValid(data_in) && allocMem(data_out, getDataSize(data_in) + delay)) {
            for (int i = 0; i < getDataSize(data_in); ++i) {
                getSamples(data_out)[i] += getSamples(data_in)[i];
                getSamples(data_out)[i + delay] += getSamples(data_in)[i] * coef;
            }
            copySndInfo(data_in, data_out);
            getSndInfo(data_out).frames += delay;
        }
    }
}
