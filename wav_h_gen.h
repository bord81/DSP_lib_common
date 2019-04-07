//
// Created by bobr on 3/28/19.
//

#ifndef DSP_LIB_WAV_H_GEN_H
#define DSP_LIB_WAV_H_GEN_H

#include <memory>

struct wav_header {
    char chunk_id[4] = {'R', 'I', 'F', 'F'};
    char chunk_format[4] = {'W', 'A', 'V', 'E'};

    char sub_chunk_1_id[4] = {'f', 'm', 't', '\x20'};
    char sub_chunk_1_size[4] = {'\x10', '\0', '\0', '\0'};
    char audio_format[2] = {'\x01', '\0'};
    char num_channels[2] = {'\x01', '\0'};
    char block_align[2] = {'\x02', '\0'};
    char bits_sample[2] = {'\x20', '\0'};

    char sub_chunk_2_id[4] = {'d', 'a', 't', 'a'};

    uint32_t sub_chunk_2_size = 0;
    uint32_t chunk_size = 0;
    uint32_t sample_rate = 0;
    uint32_t byte_rate = 0;
};

class wav_h_gen {
public:
    wav_h_gen();
    wav_h_gen(const wav_h_gen &w);
    ~wav_h_gen();

    wav_header *get_wav_header(int samples_count, int sample_rate);

private:
    wav_header *header;
};

#endif //DSP_LIB_WAV_H_GEN_H
