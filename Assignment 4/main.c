#include "usart.c"

void AC_init()
{
    // Step 1
    // Set pin PD2 (port D, pin 2) as an input
    PORTD.DIRCLR = PIN2_bm;
    // Disable digital input buffer and pull-up resistor for pin PD2
    PORTD.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;

    PORTA.PINCTRLUPD = 0xFF; // Update all pins on PORT A
    PORTB.PINCTRLUPD = 0xFF; // Update all pins on PORT B
    PORTC.PINCTRLUPD = 0xFF; // Update all pins on PORT C
    // PORTD.PINCTRLUPD = 0xFF;  // Update all pins on PORT D
    PORTE.PINCTRLUPD = 0xFF; // Update all pins on PORT E

    // Step 2
    // Select the positive and negative input sources
    AC0.MUXCTRL = AC_MUXPOS_AINP0_gc | AC_MUXNEG_DACREF_gc;

    // Step 3 & 4
    // Enable output and the Analog Comparator
    AC0.CTRLA = AC_OUTEN_bm | AC_ENABLE_bm;

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
    USART3_Init();

    while (1)
    {
        // Implement the busy-waiting scheme
        if (AC_above_threshold())
        {
            // USART3_SendString("AC above threshold\n");
            set_LED_off();
        }
        else
        {
            // USART3_SendString("AC below threshold\n");
            set_LED_on();
        }
    }
    return 0;
}