#include "usart.h"
#include <avr/sleep.h>
#include <avr/power.h>

void AC_init()
{
    // Set floating input pins to input with pull-up
    PORTA.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTA.PINCTRLUPD = 0xFF; // Update all pins on PORT B
    PORTB.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTB.PINCTRLUPD = 0xFF; // Update all pins on PORT B
    PORTC.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTC.PINCTRLUPD = 0xFF; // Update all pins on PORT C
    PORTD.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTD.PINCTRLUPD = 0xFF; // Update all pins on PORT E
    PORTE.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTE.PINCTRLUPD = 0xFF; // Update all pins on PORT E

    // Step 1
    // Set pin PD2 (port D, pin 2) as an input
    PORTD.DIRCLR = PIN2_bm;
    // Disable digital input buffer and pull-up resistor for pin PD2
    PORTD.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;

    // Step 2
    AC0.MUXCTRL = AC_MUXPOS_AINP0_gc | AC_MUXNEG_DACREF_gc;

    // Step 3
    AC0.CTRLA = AC_POWER_PROFILE2_gc | AC_RUNSTDBY_bm | AC_ENABLE_bm;
    AC0.CTRLB = AC_WINSEL_DISABLED_gc;

    // Set the DAC reference
    // DACREF = (0.1 / 1.024) * 256 = 25
    AC0.DACREF = 25;

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

int main()
{
    AC_init();
    VREF_init();
    LED_init();

    set_sleep_mode(SLEEP_MODE_STANDBY); // Standby sleep mode
    sleep_mode();

    while (1)
    {
    }

    return 0;
};