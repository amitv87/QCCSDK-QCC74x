#ifndef __ES8388_I2S_H__
#define __ES8388_I2S_H__

/* es8388 role */
#define ES8388_SLAVE  0x0 /*!< Slave Mode */
#define ES8388_MASTER 0x1 /*!< Master Mode */

/* es8388 work mode */
#define ES8388_CODEC_MDOE     0x0 /*!< ES8388 work at codec mode */
#define ES8388_RECORDING_MODE 0x1 /*!< ES8388 work at recording mode */
#define ES8388_PLAY_BACK_MODE 0x2 /*!< ES8388 work at paly back mode */
#define ES8388_BY_PASS_MODE   0x3 /*!< ES8388 work at by pass mode */

/* es8388 mic input mode */
#define ES8388_SINGLE_ENDED_MIC 0x0 /*!< Mic Single Input Mode */
#define ES8388_DIFF_ENDED_MIC   0x1 /*!< Mic Different Input Mode */

/* es8388 min pga */
#define ES8388_MIC_PGA_0DB  0x0 /*!< Mic PGA as 0db */
#define ES8388_MIC_PGA_3DB  0x1 /*!< Mic PGA as 3db */
#define ES8388_MIC_PGA_6DB  0x2 /*!< Mic PGA as 6db */
#define ES8388_MIC_PGA_9DB  0x3 /*!< Mic PGA as 9db */
#define ES8388_MIC_PGA_12DB 0x4 /*!< Mic PGA as 12db */
#define ES8388_MIC_PGA_15DB 0x5 /*!< Mic PGA as 15db */
#define ES8388_MIC_PGA_18DB 0x6 /*!< Mic PGA as 18db */
#define ES8388_MIC_PGA_21DB 0x7 /*!< Mic PGA as 21db */
#define ES8388_MIC_PGA_24DB 0x8 /*!< Mic PGA as 24db */

/* es8388 frame mode */
#define ES8388_STD_I2S_FRAME       0x0 /*!< Standard I2S Frame */
#define ES8388_LEFT_JUSTIFY_FRAME  0x1 /*!< Left  Justify Frame */
#define ES8388_RIGHT_JUSTIFY_FRAME 0x2 /*!< Right Justify Frame */
#define ES8388_DSP_FRAME           0x3 /*!< DSP Frame */

/* es8388 data witdh */
#define ES8388_DATA_LEN_24 0x0 /*!< I2S Auido Data Len 24 */
#define ES8388_DATA_LEN_20 0x1 /*!< I2S Auido Data Len 20 */
#define ES8388_DATA_LEN_18 0x2 /*!< I2S Auido Data Len 18 */
#define ES8388_DATA_LEN_16 0x3 /*!< I2S Auido Data Len 16 */
#define ES8388_DATA_LEN_32 0x4 /*!< I2S Auido Data Len 32 */

struct es8388_cfg {
    uint8_t work_mode;      /*!< ES8388 work mode */
    uint8_t role;           /*!< ES8388 role */
    uint8_t mic_input_mode; /*!< ES8388 mic input mode */
    uint8_t mic_pga;        /*!< ES8388 mic PGA */
    uint8_t i2s_frame;      /*!< ES8388 I2S frame */
    uint8_t data_width;     /*!< ES8388 I2S dataWitdh */
};

int es8388_init(void *cfg);
void es8388_reg_dump(void);
int es8388_set_volume(int volume);
int es8388_play(void *cfg);

#endif /* __ES8388_I2S_H__ */
