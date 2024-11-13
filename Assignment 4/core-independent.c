#include <xc.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
// #include "usart.h"
#include <avr/power.h>

// 256*100/1024
#define DACREF_VALUE 25

void AC_init()
{
    PORTA.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTA.PINCTRLUPD = 0xFF;
    PORTC.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTC.PINCTRLUPD = 0xFF;
    PORTD.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTD.PINCTRLUPD = 0xFF;
    // Set pin PD2 (port D, pin 2) as an input
    PORTD.DIRCLR = PIN2_bm;
    // Disable digital input buffer and pull-up resistor for pin PD2
    PORTD.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
    // Remaining initialization steps...
    AC0.MUXCTRL = AC_MUXPOS_AINP0_gc |
                  AC_MUXNEG_DACREF_gc;
    AC0.CTRLA = AC_POWER_PROFILE2_gc |
                AC_RUNSTDBY_bm |
                AC_ENABLE_bm;
    AC0.DACREF = DACREF_VALUE;
    AC0.CTRLB = AC_WINSEL_DISABLED_gc;
    AC0.INTCTRL = 0;
    EVSYS.CHANNEL0 = EVSYS_CHANNEL0_AC0_OUT_gc;
}

void VREF_init(void)
{
    VREF.ACREF = VREF_REFSEL_1V024_gc;
}

void LED_init()
{
    PORTA.DIRSET = PIN2_bm;
    EVSYS.USEREVSYSEVOUTA = 1;
}
void set_LED_on()
{
    // LED is active low. Set pin LOW to turn LED on
    PORTA.OUTCLR = PIN2_bm;
}
void set_LED_off()
{
    // LED is active low. Set pin HIGH to turn LED off
    PORTA.OUTSET = PIN2_bm;
}

// Function to initialize sleep mode
void sleep_init(void)
{
    set_sleep_mode(SLEEP_MODE_STANDBY); // Standby sleep mode
}

int main()
{
    AC_init();
    VREF_init();
    LED_init();
    // USART3_Init();

    sleep_init();
    sleep_mode();
    while (1)
    {
    }
    return 0;
};