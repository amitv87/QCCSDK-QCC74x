#include <stdint.h>
#include <qcc743_glb.h>
#include <qcc743.h>
#include <mfg_sdio.h>
#include <mfg_sdu.h>

/*SDIO CLK*/
#define QCC74x_BOOT_SDIO_CLK0_GPIO            GLB_GPIO_PIN_0
#define QCC74x_BOOT_SDIO_CLK0_INPUT_FUN       GPIO0_FUN_SWGPIO_0
#define QCC74x_BOOT_SDIO_CLK0_NORMAL_FUN      GPIO0_FUN_SDIO_CLK
/*SDIO CMD*/
#define QCC74x_BOOT_SDIO_CMD0_GPIO            GLB_GPIO_PIN_1
#define QCC74x_BOOT_SDIO_CMD0_INPUT_FUN       GPIO1_FUN_SWGPIO_1
#define QCC74x_BOOT_SDIO_CMD0_NORMAL_FUN      GPIO1_FUN_SDIO_CMD
/*SDIO DATA0*/
#define QCC74x_BOOT_SDIO_DATA00_GPIO          GLB_GPIO_PIN_2
#define QCC74x_BOOT_SDIO_DATA00_INPUT_FUN     GPIO1_FUN_SWGPIO_1
#define QCC74x_BOOT_SDIO_DATA00_NORMAL_FUN    GPIO2_FUN_SDIO_DAT0
#define QCC74x_BOOT_SDIO_DATA10_GPIO          GLB_GPIO_PIN_3
#define QCC74x_BOOT_SDIO_DATA10_INPUT_FUN     GPIO3_FUN_SWGPIO_3
#define QCC74x_BOOT_SDIO_DATA10_NORMAL_FUN    GPIO3_FUN_SDIO_DAT1
#define QCC74x_BOOT_SDIO_DATA20_GPIO          GLB_GPIO_PIN_4
#define QCC74x_BOOT_SDIO_DATA20_INPUT_FUN     GPIO4_FUN_SWGPIO_4
#define QCC74x_BOOT_SDIO_DATA20_NORMAL_FUN    GPIO4_FUN_SDIO_DAT2
#define QCC74x_BOOT_SDIO_DATA30_GPIO          GLB_GPIO_PIN_5
#define QCC74x_BOOT_SDIO_DATA30_INPUT_FUN     GPIO5_FUN_SWGPIO_5
#define QCC74x_BOOT_SDIO_DATA30_NORMAL_FUN    GPIO5_FUN_SDIO_DAT3

#define WL_REGS8(x)     	(*(volatile unsigned char *)(x))
#define SDIO_RX_BUF_SIZE	2048
#define SDIO_TX_BUF_SIZE	2048
#define SDIO_MAX_PORT_NUM   1
#define SDU_INT_HOST        1
#define SDU_SEND_TIME_OUT   2000

uint32_t __attribute__((aligned(4)))  bootrom_readbuf[2][(1024 * 4 + 3) / 4] __attribute__ ((section(".wifibss"))); //
uint32_t bootrom_cmd_ack_buf[16];
volatile uint32_t rx_buf_index=0;
qcc74x_sdio_read_cb_t gl_read_callback = NULL;
uint8_t __attribute__((aligned(4))) gbuf[2050] __attribute__ ((section(".wifibss")));
int g_data_flag = 1;

/* Check whether rx buf has been attached to one port by corresponding bit */
static volatile uint16_t sdu_rx_buf_attached[NUM_FUNC];

void sdu_interrupt_entry(void *arg);


static void sdu_attach_rx_bufs(uint8_t fn_num,uint8_t port_num)
{
    /* use only one port */
    bootrom_readbuf[rx_buf_index%2][0]=0xffffffff;
    SdioFuncReg[fn_num]->WrIdx = port_num;
    SdioFuncReg[fn_num]->SqWriteBase = (uint32_t)&bootrom_readbuf[rx_buf_index%2];
    SdioFuncReg[fn_num]->WrBitMap = (1<<port_num);
    sdu_rx_buf_attached[fn_num] |= (1<<port_num);

#if SDU_INT_HOST
    /* Generate host sdio interrupt */
    sdio_GEN_CARD2HOST_INT(fn_num,(SDIO_CCR_CS_ReadCISRdy | SDIO_CCR_CS_DnLdRdy  | SDIO_CCR_CS_IORdy));
#endif
}

uint8_t sdu_read_s_reg(uint32_t offset)
{
    return WL_REGS8(QCC74x_FUNC_SCRATCH_BASE + offset);
}

void sdu_write_s_reg(uint32_t offset, const uint8_t val)
{
    WL_REGS8(QCC74x_FUNC_SCRATCH_BASE + offset) = val;
}

int32_t sdu_host_check(void)
{
    uint16_t blockSize;
    //uint8_t powerUp = WL_REGS8(QCC74x_FUNC_SCRATCH_BASE);

    //WL_REGS8(QCC74x_SDIO_HS_SUPPORT)=(WL_REGS8(QCC74x_SDIO_HS_SUPPORT)&0xfe);
    /* Wait for host driver to set register 0x60 of func1 as 1 */
    //if(!powerUp){//read register, if val is 1, power up, shake hands success.
    //	return -1;
    //}
    printf("SDIO Host write\r\n");
    WL_REGS8(QCC74x_FUNC_SCRATCH_BASE)=0;    //clear power value;
    blockSize = WL_REGS8(SDIO_FN1_BLK_SIZE_0); //read reg, get blockSize0
    blockSize |= ((WL_REGS8(SDIO_FN1_BLK_SIZE_1) &
                   SDIO_FN1_BLK_SIZE_1_MASK ) << 8);

    SdioFuncReg[FUNC_WIFI]->RdBitMap = 0x0;
    SdioFuncReg[FUNC_WIFI]->WrBitMap = 0x0;//reset rd and wr bitmap
    /* toggle SDIO_CCR_CIC_DnLdOvr on WL_SDIO_CCR_CARD_INT_CAUSE */
    SdioFuncReg[FUNC_WIFI]->CardIntStatus = SDIO_CCR_CIC_DnLdOvr;//?
    SdioFuncReg[FUNC_WIFI]->CardIntStatus = 0;///clear int

    //if (!flag_mport[FUNC_WIFI]){
        // It's not needed to enable the registers if only using single port
        //Should the host instruct what to set?
        SdioFuncReg[FUNC_WIFI]->Config2 |= CONFIG2_MSK ;
        /* Enable Bit 4 in config register, to receive interrupt as
           soon as the aggregation/deaggregation is complete for a single
           packet instead of waiting for the complete aggregation/deaggration
           to complete.
        */
        SdioFuncReg[FUNC_WIFI]->Config |= 0x00000010;
    //}

    /* unmask the interrupts */
    //SdioFuncReg[FUNC_WIFI]->CardIntMask = SDIO_CCR_CIM_MASK;
    SdioFuncReg[FUNC_WIFI]->CardIntMask = 0x0067;
   /* select interrupt reset mode */
    SdioFuncReg[FUNC_WIFI]->CardIntMode = 0;
    /* disable sdu interrupt*/

    sdu_attach_rx_bufs(FUNC_WIFI,0);
#if 0
    if (SdioFuncReg[FUNC_WIFI]->WrBitMap & SDIO_DATA_PORTS_MASK)
    {
        sdio_GEN_CARD2HOST_INT(FUNC_WIFI,(SDIO_CCR_CS_ReadCISRdy | SDIO_CCR_CS_DnLdRdy  | SDIO_CCR_CS_IORdy));
    }
    else
    {
        sdio_GEN_CARD2HOST_INT(FUNC_WIFI,(SDIO_CCR_CS_ReadCISRdy | SDIO_CCR_CS_IORdy));
    }
#endif

    return 0;
}

void sdu_deinit(void)
{
    uint8_t value;

    value = SdioFuncReg[FUNC_WIFI]->CardIntStatus;
    /* Clear Interrupt Bit */
    SdioFuncReg[FUNC_WIFI]->CardIntStatus = ~value|SDIO_CCR_CIC_PwrUp;

    //NVIC_ClearPendingIRQ(SDIO_IRQn);
    qcc74x_irq_clear_pending(SDIO_IRQn);
    //GLB_AHB_Slave1_Reset(QCC74x_AHB_SDU);
}

int sdu_send_data(uint32_t *data,uint32_t len)
{
    /* The port number for upload packets*/
    static uint8_t curr_upld_port = 0;

    uint8_t fn = 0;
    uint8_t value;
    /* uint64_t count_time; */
    static int count = 0;
    uint32_t bitmap;

    value = SdioFuncReg[fn]->CardIntStatus;
    while(!(value & SDIO_CCR_CIC_UpLdOvr)) {
        if (count == 0) {
            break;
        }
    }

    while(1) {
        bitmap = SdioFuncReg[fn]->RdBitMap;
        if (bitmap == 0) {
            break;
        }
    }

    if (value & SDIO_CCR_CIC_UpLdOvr || count == 0)
    {
        count = 1;
        //SdioFuncReg[fn]->CardIntStatus = ~value|SDIO_CCR_CIC_PwrUp;
        SdioFuncReg[fn]->CardIntStatus =  value & 0xfd;
        if (!(SdioFuncReg[fn]->RdBitMap & (1<< curr_upld_port)))
        {
            SdioFuncReg[0]->RdIdx = curr_upld_port;
            SdioFuncReg[0]->RdLen[curr_upld_port] = len;
            SdioFuncReg[0]->SqReadBase = (uint32_t)&data[0];
            SdioFuncReg[fn]->RdBitMap = 1 << curr_upld_port;
            sdio_GEN_CARD2HOST_INT(0, SDIO_CCR_CS_UpLdRdy);
            /* count_time = qcc74x_timer_now_us64(); */
              /* wait for data send finished */
            volatile uint32_t to_cnt = 0xaffff;
            while((SdioFuncReg[fn]->RdBitMap & (1<< curr_upld_port))){
                /* if((qcc74x_timer_now_us64() - count_time) / 1000 > SDU_SEND_TIME_OUT){ */
                /*  	printf("sdu send timeout\r\n"); */
                /*      return -1; */
                /* } */
                if (to_cnt == 0) {
                    return -1;
                }
                to_cnt--;
            }
        }
    }

    return 0;
}


uint32_t *sdu_receive_data(uint32_t recv_len, uint8_t int_status)
{
    /* The port number for host to download packets*/
    static uint8_t curr_dnld_port = 0;
    uint8_t fn;
    /* uint8_t value; */
    uint8_t CRCError;
    uint32_t *recv_buf=NULL;

    for (fn = 0; fn < NUM_FUNC; fn++)
    {
        switch(fn)
        {
            case FUNC_WIFI:
            {
                /* value = SdioFuncReg[fn]->CardIntStatus; */
                /* Read the CRC error for the CMD 53 write*/
                CRCError = SdioFuncReg[fn]->HostTransferStatus;

                if (int_status & SDIO_CCR_CIC_DnLdOvr)
                {
                    /* Clear Interrupt Bit */
                    //SdioFuncReg[fn]->CardIntStatus = ~value|SDIO_CCR_CIC_PwrUp;
                    SdioFuncReg[fn]->CardIntStatus = int_status & 0xfe;
                    if(CRCError & SDIO_CCR_HOST_INT_DnLdCRC_err)
                    {
                        break;
                    }
                    if (!(SdioFuncReg[fn]->WrBitMap & (1<< curr_dnld_port)) &&
                        (sdu_rx_buf_attached[fn] & (1<< curr_dnld_port)))
                    {
                        /*application data format:cmdID+rsvd+dataLen0+dataLen1+...*/
                        recv_buf=(uint32_t *)&bootrom_readbuf[rx_buf_index%2];
                        /*clear current download port attach flag*/
                        sdu_rx_buf_attached[fn] &= ~(1<< curr_dnld_port);///clear attached flag
                        rx_buf_index++;
                        sdu_attach_rx_bufs(fn,curr_dnld_port);
                        /*move on to next port*/
                        //curr_dnld_port++;
                        //if(curr_dnld_port == SDIO_MAX_PORT_NUM){
                        //    curr_dnld_port = 0;
                        //}
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    return recv_buf;
}

void gen_card2host_int(void)
{
    //sdio_GEN_CARD2HOST_INT(0,(SDIO_CCR_CS_ReadCISRdy | SDIO_CCR_CS_DnLdRdy  | SDIO_CCR_CS_IORdy));
    //GLB_GPIO_Write(GLB_GPIO_PIN_3, 1);
    sdio_GEN_CARD2HOST_INT(0, SDIO_CCR_CS_UpLdRdy);
    //sdio_GEN_CARD2HOST_INT(1, SDIO_CCR_CS_UpLdRdy);
    //GLB_GPIO_Write(GLB_GPIO_PIN_3, 0);


    return;
}

int qcc74x_sdu_init(void)
{
    //GLB_GPIO_Cfg_Type cfg;
    //uint8_t gpiopins[6];
    //uint8_t gpiofuns[6];
    //uint8_t i=0;

    // NOT resetting here because reset may cause host to stop powering
#if 0
    cfg.gpioMode=GPIO_MODE_AF;
    cfg.pullType=GPIO_PULL_NONE;
    cfg.drive=1;
    cfg.smtCtrl=1;

    gpiopins[0]=QCC74x_BOOT_SDIO_CLK0_GPIO;
    gpiopins[1]=QCC74x_BOOT_SDIO_CMD0_GPIO;
    gpiopins[2]=QCC74x_BOOT_SDIO_DATA00_GPIO;
    gpiopins[3]=QCC74x_BOOT_SDIO_DATA10_GPIO;
    gpiopins[4]=QCC74x_BOOT_SDIO_DATA20_GPIO;
    gpiopins[5]=QCC74x_BOOT_SDIO_DATA30_GPIO;
    gpiofuns[0]=QCC74x_BOOT_SDIO_CLK0_NORMAL_FUN;
    gpiofuns[1]=QCC74x_BOOT_SDIO_CMD0_NORMAL_FUN;
    gpiofuns[2]=QCC74x_BOOT_SDIO_DATA00_NORMAL_FUN;
    gpiofuns[3]=QCC74x_BOOT_SDIO_DATA10_NORMAL_FUN;
    gpiofuns[4]=QCC74x_BOOT_SDIO_DATA20_NORMAL_FUN;
    gpiofuns[5]=QCC74x_BOOT_SDIO_DATA30_NORMAL_FUN;

    for(i=0;i<sizeof(gpiopins);i++){
        cfg.gpioPin=gpiopins[i];
        cfg.gpioFun=gpiofuns[i];
        if(i==0){
            /*sdio clk is input*/
            cfg.gpioMode=GPIO_MODE_INPUT;
        }else{
            /*data are bidir*/
            cfg.gpioMode=GPIO_MODE_AF;
        }
        GLB_GPIO_Init(&cfg);
    }

    GLB_AHB_Slave1_Reset(QCC74x_AHB_SLAVE1_SDU);
#endif
    qcc74x_irq_attach(SDIO_IRQn, sdu_interrupt_entry, NULL);
    qcc74x_irq_enable(SDIO_IRQn);

    return 0;
}

int32_t qcc74x_sdio_handshake(void)
{
    int32_t ret=0;

    ret = sdu_host_check();
    if(ret >= 0){
        return 0;
    }

	return -1;
}

int qcc74x_sdio_read_cb_register(void *env, qcc74x_sdio_read_cb_t cb, const void *cb_arg)
{
    if (cb == NULL) {
        printf("cb can not be NULL \r\n");

        return -1;
    }

    gl_read_callback = cb;

    return 0;
}

int qcc74x_hal_sdio_write(void *env, const void *data_ptr, const uint16_t data_len)
{
    uint8_t hdata;
    uint8_t ldata;

    ldata = data_len & 0xff;
    hdata = (data_len >> 8) & 0xff;

    gbuf[0] = ldata;
    gbuf[1] = hdata;
    memcpy(&gbuf[2], data_ptr, data_len);
	return sdu_send_data((uint32_t *)gbuf, SDIO_TX_BUF_SIZE);
}

void sdu_interrupt_entry(void *arg)
{
    uint16_t len;
    uint8_t *pbuf;
    uint8_t value;

    value = SdioFuncReg[0]->CardIntStatus;
    if (value & SDIO_CCR_CIC_DnLdOvr) {
        pbuf = (uint8_t *)sdu_receive_data(SDIO_RX_BUF_SIZE, value);
        if (!pbuf) {
            return;
        }
        len = pbuf[0] | (pbuf[1] << 8);
        if (gl_read_callback != NULL) {
            gl_read_callback(NULL, pbuf + 2, len);
        }
    }

    if (value & SDIO_CCR_CIC_UpLdOvr) {

    }

    return;
}
