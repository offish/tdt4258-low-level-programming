#include "usart.h"
#include <avr/sleep.h>
#include <avr/interrupt.h>

void AC_init()
{
    // Set floating input pins to input with pull-up
    PORTB.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTB.PINCTRLUPD = 0xFF; // Update all pins on PORT B
    PORTC.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
    PORTC.PINCTRLUPD = 0xFF; // Update all pins on PORT C
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

void TCA0_init()
{
    // Set the period of the timer. PER = period[s] * F_CPU / Prescaler = 0.01s * 4 000 000 Hz / 2
    TCA0.SINGLE.PER = 20000;
    // Enable timer overflow interrupt
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
    // Run timer in standby mode, set prescaler to 2, enable timer
    TCA0.SINGLE.CTRLA = TCA_SINGLE_RUNSTDBY_bm | TCA_SINGLE_CLKSEL_DIV2_gc | TCA_SINGLE_ENABLE_bm;
}

bool AC_above_threshold()
{
    // Check the output of the Analog Comparator
    return (bool)(AC0.STATUS & AC_CMPSTATE_bm);
}

ISR(TCA0_OVF_vect)
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
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

int main()
{
    AC_init();
    VREF_init();
    LED_init();

    // Initialize timer
    TCA0_init();

    // Enable interrupts
    sei();

    set_sleep_mode(SLEEP_MODE_STANDBY); // Standby sleep mode

    while (1)
    {
        sleep_mode();
    }

    return 0;
};