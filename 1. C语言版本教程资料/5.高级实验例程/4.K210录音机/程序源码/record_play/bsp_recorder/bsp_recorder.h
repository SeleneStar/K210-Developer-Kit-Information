#ifndef _BSP_RECORDER_H_
#define _BSP_RECORDER_H_

#include "stdint.h"
#include "ff.h"


typedef struct _wave_header_t 
{
    uint8_t chunkID[4];
    uint32_t chunkSize;
    uint8_t format[4];

    uint8_t subchunk1ID[4];
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;

    uint8_t subchunk2ID[4];
    uint32_t subchunk2Size;
} __attribute__((packed, aligned(4))) wave_header_t; 


wave_header_t *creat_wave_header(uint32_t length, uint32_t sample_rate, uint8_t bits_perSample, uint8_t num_chans);
void update_length(wave_header_t *header, uint32_t length_bytes);

void recoder_new_pathname(uint8_t *pname);
void recoder_get_last_pathname(uint8_t *pname);


void bsp_recorder_init(void);
void bsp_recorder_start(TCHAR *path);
void bsp_recorder_stopsave(TCHAR *path);

void bsp_record_save_sd(void);

#endif /* _WAVE_HEADER_H_ */
