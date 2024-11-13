#include "usart.h"

void AC_init()
{
    // Step 1
    // Set pin PD2 (port D, pin 2) as an input
    PORTD.DIRCLR = PIN2_bm;
    // Disable digital input buffer and pull-up resistor for pin PD2
    PORTD.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;

    // Step 2
    // Select the positive and negative input sources
    AC0.MUXCTRL = AC_MUXPOS_AINP0_gc | AC_MUXNEG_DACREF_gc;

    // Step 3 & 4
    // Enable output and the Analog Comparator
    AC0.CTRLA = AC_OUTEN_bm | AC_ENABLE_bm;
}

void VREF_init(void)
{
    VREF.ACREF = VREF_REFSEL_1V024_gc;
    VREF.DAC0REF = 25 / 256 * VREF_REFSEL_1V024_gc;

    // AC0.DACREF = 0x18;
}

void LED_init()
{
    PORTA.DIRSET = PIN2_bm;
    // PORTA.PIN2CTRL = PORT_PINCTRLSET_7_bm;
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

int main()
{
    AC_init();
    VREF_init();
    LED_init();

    while (1)
    {
        // Implement the busy-waiting scheme
        if (AC_above_threshold())
        {
            set_LED_on();
        }
        else
        {
            set_LED_off();
        }
    }
    return 0;
}