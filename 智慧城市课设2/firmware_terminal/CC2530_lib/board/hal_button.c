/***********************************************************************************
    Filename:     hal_button.c

    Description:  HAL button implementation for CCSoC + SmartRF05EB

    Copyright 2007 Texas Instruments, Inc.
***********************************************************************************/
#include "hal_board.h"
#include "hal_button.h"
#include "hal_cc8051.h"


/******************************************************************************
* @fn  halButtonInit
*
* @brief
*      Initializes the button functionality
*
* Parameters:
*
* @param  void
*
* @return void
*
******************************************************************************/
void halButtonInit(void)
{
    // Button push input
    MCU_IO_INPUT(HAL_BOARD_IO_BTN_1_PORT, HAL_BOARD_IO_BTN_1_PIN, MCU_IO_TRISTATE);
    MCU_IO_INPUT(HAL_BOARD_IO_BTN_2_PORT, HAL_BOARD_IO_BTN_2_PIN, MCU_IO_TRISTATE);
}


/******************************************************************************
* @fn  halButtonPushed
*
* @brief
*      This function detects if 'S1' is being pushed. The function
*      implements software debounce. Return true only if previuosly called
*      with button not pushed. Return true only once each time the button
*      is pressed.
*
* Parameters:
*
* @param  void
*
* @return uint8
*          1: Button is being pushed
*          0: Button is not being pushed
*
******************************************************************************/
uint8 halButtonPushed(void)
{
    extern volatile uint8 led4State;

    uint8        v= HAL_BUTTON_NONE;


    // Need to set direction because the button is shared with LED4 on SmartRF05EB rev 1.7+
    MCU_IO_INPUT(HAL_BOARD_IO_BTN_1_PORT, HAL_BOARD_IO_BTN_1_PIN, MCU_IO_TRISTATE);
    MCU_IO_INPUT(HAL_BOARD_IO_BTN_2_PORT, HAL_BOARD_IO_BTN_2_PIN, MCU_IO_TRISTATE);

    if (0==HAL_BUTTON_1_PUSHED()) {
        HAL_DEBOUNCE(HAL_BUTTON_1_PUSHED());
        return HAL_BUTTON_1;
    }

    if (0==HAL_BUTTON_2_PUSHED()) {
        HAL_DEBOUNCE(HAL_BUTTON_2_PUSHED());
        return HAL_BUTTON_2;
    }

    return v;
}
