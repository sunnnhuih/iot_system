#include "hal_defs.h"
#include "hal_cc8051.h"
#include "hal_mcu.h"
#include "hal_digio.h"
#include "hal_int.h"
#include "hal_board.h"
#include "hal_uart.h"

void halBoardInit(void)
{
    halMcuInit();//MCU初始化，32M,32K
    halLedinit();//LED初始化
    HalLcdInit();//LCD初始化
    halButtonInit();//按键
    halUartInit(115200);//串口初始化
    halIntOn();//打开中断
}
