#include "wav_h_gen.h"

wav_h_gen::wav_h_gen() {
    header = new wav_header;
}

wav_h_gen::wav_h_gen(const wav_h_gen &w) {
    header = new wav_header;
    *header = *w.header;
}

wav_h_gen::~wav_h_gen() {
    delete header;
}

wav_header *wav_h_gen::get_wav_header(int samples_count, int sample_rate) {
    header->sub_chunk_2_size = static_cast<uint32_t>(samples_count * 2);
    header->chunk_size = 36 + header->sub_chunk_2_size;
    header->sample_rate = static_cast<uint32_t>(sample_rate);
    header->byte_rate = static_cast<uint32_t>(header->sample_rate * 2);
    return header;
}
