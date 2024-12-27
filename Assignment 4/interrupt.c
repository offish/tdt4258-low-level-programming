#include "usart.h"
#include <avr/sleep.h>
#include <avr/interrupt.h>

void AC_init()
{
    // Step 1
    // Set pin PD2 (port D, pin 2) as an input
    PORTD.DIRCLR = PIN2_bm;
    // Disable digital input buffer and pull-up resistor for pin PD2
    PORTD.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;

    // Step 2
    AC0.MUXCTRL = AC_MUXPOS_AINP0_gc | AC_MUXNEG_DACREF_gc;

    // Step 3 & 4
    AC0.CTRLA = AC_POWER_PROFILE2_gc | AC_RUNSTDBY_bm | AC_ENABLE_bm;

    // Set the DAC reference
    // DACREF = (0.1 / 1.024) * 256 = 25
    AC0.DACREF = 25;

    AC0.INTCTRL = AC_CMP_bm;
}

void VREF_init(void)
{
    VREF.ACREF = VREF_REFSEL_1V024_gc;
}

void LED_init()
{
    PORTA.DIRSET = PIN2_bm;
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

bool AC_above_threshold()
{
    // Check the output of the Analog Comparator
    return (bool)(AC0.STATUS & AC_CMPSTATE_bm);
}

ISR(AC0_AC_vect)
{
    if (AC_above_threshold())
    {
        set_LED_off();
    }
    else
    {
        set_LED_on();
    }

    // Clear interrupt flag
    AC0.STATUS = AC_CMPIF_bm;
}

int main()
{
    AC_init();
    VREF_init();
    LED_init();

    // Enable interrupts
    sei();

    set_sleep_mode(SLEEP_MODE_STANDBY); // Standby sleep mode

    while (1)
    {
        sleep_mode();
    }

    return 0;
};