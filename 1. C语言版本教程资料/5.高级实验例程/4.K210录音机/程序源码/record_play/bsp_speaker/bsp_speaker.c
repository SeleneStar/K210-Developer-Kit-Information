#include "gpiohs.h"
#include "bsp_speaker.h"
#include "pin_config.h"
#include "wav_decode.h"
#include "stdio.h"
#include "i2s.h"
#include "rgb.h"
#include "iomem.h"

#define PLAY_I2S_DMA_CHANNEL            DMAC_CHANNEL5

wav_file_t * wav_file;

int g_speaker_switch = 0, g_speaker_times = 0;
FIL* file = NULL;


void bsp_speaker_deinit(void);
void wav_decode_finish(void);

static int on_play_dma_cb(void *ctx)
{
    int status = 0;
     //解码
    if(g_speaker_switch == 1)
    {
        //enum errorcode_e status;
        status = wav_decode(wav_file);
       
        if(FILE_FAIL == status)
        {
            printf("decode fail\r\n");
            f_close(file);
            f_close(wav_file->fp);
            wav_decode_finish();
            g_speaker_switch = 0;
            app_rgb_blue_state(LIGHT_OFF);
            return -1;
        }

    }
    //读取文件
    if(wav_file->buff_end)
    {
        wav_file->buff_end = 2;
        return 0;
    }
    if(wav_file->buff_index == 0)
    {
        if(wav_file->buff1_used == 0)
        {
            printf("on_play_dma_cb: error1\r\n");
            bsp_speaker_deinit();
            wav_decode_finish();
            app_rgb_blue_state(LIGHT_OFF);
            return 0;
        }
        wav_file->buff0_used = 0;
        wav_file->buff_index = 1;
        wav_file->buff_current = wav_file->buff1;
        wav_file->buff_current_len = wav_file->buff1_read_len;
        if(wav_file->buff1_len > wav_file->buff1_read_len)
            wav_file->buff_end = 1;
    } 
    else if(wav_file->buff_index == 1)
    {
        if(wav_file->buff0_used == 0)
        {
            printf("on_play_dma_cb: error2\r\n");
            bsp_speaker_deinit();
            wav_decode_finish();
            app_rgb_blue_state(LIGHT_OFF);
            return 0;
        }
        wav_file->buff1_used = 0;
        wav_file->buff_index = 0;
        wav_file->buff_current = wav_file->buff0;
        wav_file->buff_current_len = wav_file->buff0_read_len;
        if(wav_file->buff0_len > wav_file->buff0_read_len)
            wav_file->buff_end = 1;
    }


    //播放
    i2s_play(I2S_DEVICE_2,
             PLAY_I2S_DMA_CHANNEL,
             (void *)wav_file->buff_current,
             wav_file->buff_current_len,
             wav_file->buff_current_len,
             16, 2);

    if(FILE_END == status)
    {
        bsp_speaker_deinit();  //释放dma中断
        printf("decode finish\r\n");
        f_close(file);
        f_close(wav_file->fp);
        wav_decode_finish();
        g_speaker_switch = 0;
        app_rgb_blue_state(LIGHT_OFF);
        return 0;
    } 

    return 0;
}

static void bsp_speaker_init(int sample_rate)
{
    if(g_speaker_times == 0)
    {
        i2s_init(I2S_DEVICE_2, I2S_TRANSMITTER, 0xc);

        i2s_tx_channel_config(I2S_DEVICE_2, I2S_CHANNEL_1,
                            RESOLUTION_16_BIT, SCLK_CYCLES_32,
                            TRIGGER_LEVEL_4, /*TRIGGER_LEVEL_1*/
                            RIGHT_JUSTIFYING_MODE);
        i2s_set_sample_rate(I2S_DEVICE_2, sample_rate);
        g_speaker_times = 1;
    }

    dmac_set_irq(PLAY_I2S_DMA_CHANNEL, on_play_dma_cb, NULL, 2);
    i2s_play(I2S_DEVICE_2,
             PLAY_I2S_DMA_CHANNEL,
             (void *)wav_file->buff_current,
             wav_file->buff_current_len,
             wav_file->buff_current_len,
             16, 2);
}

void bsp_speaker_deinit(void)
{

    dmac_wait_done(PLAY_I2S_DMA_CHANNEL);
    //sysctl_disable_irq();
    //i2s_transmit_dma_enable_pro(I2S_DEVICE_2, 0);
    dmac_channel_disable(PLAY_I2S_DMA_CHANNEL);
    dmac_free_irq(PLAY_I2S_DMA_CHANNEL);
    //

    printf("bsp_speaker_deinit\r\n");
}

//SD卡wave文件播放
int bsp_play_wav(TCHAR *path)
{
    enum errorcode_e status;
    
    //申请FIL字节的内存空间
    file = (FIL *)iomem_malloc(sizeof(FIL));
    if(file == NULL) //申请失败
    {
        printf("bsp_speaker: bsp_play_wav file iomem_malloc failed\r\n");
        return -1;
    }

    wav_file = (wav_file_t *)iomem_malloc(sizeof(wav_file_t));
    if(wav_file == NULL) //申请失败
    {
        printf("bsp_speaker: bsp_play_wav wav_file iomem_malloc failed\r\n");
        iomem_free(file);
        app_rgb_blue_state(LIGHT_OFF);
        return -1;
    }
        
    //recoder_get_last_pathname(path);
    if(FR_OK != f_open(file, path, FA_READ))
    {
        printf("bsp_speaker: open file fail\r\n");
        f_close(file);
        iomem_free(file);
        iomem_free(wav_file);
        app_rgb_blue_state(LIGHT_OFF);
        return -1;
    }

    /* 初始化wav文件并且打印相关信息 */
    wav_file->fp = file;

    status = wav_init(wav_file);
    if(status != OK )
    {
        printf("bsp_speaker: wav_init fail, err:%d\r\n", status);
        f_close(file);
        f_close(wav_file->fp);
        iomem_free(file);
        iomem_free(wav_file);
        app_rgb_blue_state(LIGHT_OFF);
        return -1;
    }

    printf("result:%d\r\n", status);
    printf("point:0x%08x\r\n", (uint32_t)f_tell(file));
    printf("numchannels:%d\r\n", wav_file->numchannels);
    printf("samplerate:%d\r\n", wav_file->samplerate);
    printf("byterate:%d\r\n", wav_file->byterate);
    printf("blockalign:%d\r\n", wav_file->blockalign);
    printf("bitspersample:%d\r\n", wav_file->bitspersample);
    printf("datasize:%d, %08x\r\n", wav_file->datasize, wav_file->datasize);
    printf("bit_rate:%dkbps\r\n", wav_file->byterate * 8 / 1000);
    printf("start decode\r\n");

    status = wav_decode_init(wav_file);
    if(OK != status)
    {
        f_close(file);
        f_close(wav_file->fp);
        iomem_free(file);
        iomem_free(wav_file);
        printf("decode init fail\r\n");
        app_rgb_blue_state(LIGHT_OFF);
        return -1;
    }

    bsp_speaker_init(22050);
    g_speaker_switch = 1;
    app_rgb_blue_state(LIGHT_ON);
   
    return 0;
}

//释放所有动态生成的内存
void wav_decode_finish(void)
{
	iomem_free(file);
	iomem_free(wav_file->buff0);
	iomem_free(wav_file->buff1);
	iomem_free(wav_file);
	return OK;
}