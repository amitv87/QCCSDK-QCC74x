#include "qcc74x_mtimer.h"
#include "board.h"
#include "qcc743_sec_dbg.h"

static void qcc74x_show_dbg_state(void)
{
    uint32_t state;

    state = Sec_Dbg_Read_Dbg_State();

    if(state == SEC_DBG_STATE_OPEN_MODE){
        printf("Sec dbg is in open mode\r\n");
        printf("JTAG should be available\r\n");
    }else if(state == SEC_DBG_STATE_PASSWORD_MODE){
        printf("Sec dbg is in password mode, need password\r\n");
        printf("JTAG should be unavailable\r\n");
    }else if(state == SEC_DBG_STATE_CLOSE_MODE){
        printf("Sec dbg is in close mode\r\n");
    }else{
        printf("Error! Not support!\r\n");
    }
}

int main(void)
{
    uint32_t pwd[4] = {0x12345678, 0x22345678, 0x32345678, 0x42345678};

    board_init();

    printf("password case:\r\n");

    /* Password mode, JTAG can not be connected */
    qcc74x_show_dbg_state();

    qcc74x_mtimer_delay_ms(10000);

    /* Set password */
    Sec_Dbg_Set_Dbg_Pwd(pwd);
    Sec_Dbg_Set_Dbg_Trigger();
    Sec_Dbg_Wait_Ready();

    /* Open mode, JTAG can be connected */
    qcc74x_show_dbg_state();

    while (1) {
        qcc74x_mtimer_delay_ms(1000);
    }
}
