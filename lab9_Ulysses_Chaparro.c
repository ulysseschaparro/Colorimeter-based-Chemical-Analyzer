//Ulysses Chaparro Lab 9 Code

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "lab5.h"
#include "lab6.h"
#include "lab7.h"
#include "lab8.h"
#include "wait.h"
#include "rgb_led.h"
#include "adc0.h"

// PortD masks (GPI pin PD0)
#define FREQ_IN_MASK 1

#define ANALOG_MASK 32
#define BLK_MASK 16
#define W_MASK 32
#define Y_MASK 8
#define G_MASK 4

extern uint16_t pwm_r, pwm_g, pwm_b;
extern uint16_t r, g, b;
extern uint16_t raw;
extern uint16_t dc;
extern float ref1, ref2;
extern float pH;

extern float distance[5];
extern uint16_t dist_closest, dist_2ndclosest;

extern char str1[20], str2[20], str3[20];

extern uint16_t num = 0;

extern uint8_t code;

// Initialize Hardware
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R2;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R3; //PD0 timer
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1 | SYSCTL_RCGCGPIO_R4; //for ANALOG and MOTOR pins
    _delay_cycles(3);

    // Configure SIGNAL_IN for frequency and time measurements
    GPIO_PORTD_AFSEL_R |= FREQ_IN_MASK;              // select alternative functions for SIGNAL_IN pin (this says PD0 pin is peripheral signal)
    GPIO_PORTD_PCTL_R &= ~GPIO_PCTL_PD0_M;           // map alt fns to SIGNAL_IN
    GPIO_PORTD_PCTL_R |= GPIO_PCTL_PD0_WT2CCP0;
    GPIO_PORTD_DEN_R |= FREQ_IN_MASK;                // enable bit 0 for digital input

    // Configure GPIO pins (MOTOR)
    GPIO_PORTE_DIR_R |= BLK_MASK | W_MASK | Y_MASK | G_MASK;
    GPIO_PORTE_DR2R_R |= BLK_MASK | W_MASK | Y_MASK | G_MASK;
    GPIO_PORTE_DEN_R |= BLK_MASK | W_MASK | Y_MASK | G_MASK;

    // Configure AIN11 as an analog input
    GPIO_PORTB_AFSEL_R |= ANALOG_MASK;                 // select alternative functions for AIN11 (PB5)
    GPIO_PORTB_DEN_R &= ~ANALOG_MASK;                  // turn off digital operation on pin PB5
    GPIO_PORTB_AMSEL_R |= ANALOG_MASK;                 // turn on analog operation on pin PB5
}

int main(void)
{
    initHw();
    initRgb();
    initUart0();
    initAdc0Ss3();
    USER_DATA data;

    setUart0BaudRate(115200, 40e6);

    setAdc0Ss3Mux(11);
    setAdc0Ss3Log2AverageCount(2);

    enableTimerMode();

    calibrate();

    bool valid_command;
    extern bool valid;

    while(true)
    {
        if (valid)
        {
            valid = false;
            switch(code)
            {
                case 92:
                    home();
                    break;
                case 20:
                    goTo(0);
                    break;
                case 21:
                    goTo(1);
                    break;
                case 22:
                    goTo(2);
                    break;
                case 16:
                    goTo(3);
                    break;
                case 17:
                    goTo(4);
                    break;
                case 18:
                    goTo(5);
                    break;
                case 93:
                    calibrate();
                    break;
                case 84:
                    measure(0, &r, &g, &b);
                    putsUart0("\nMeasured Analog: ");
                    printValues(r, g, b);
                    break;
                case 85:
                    measure(1, &r, &g, &b);
                    putsUart0("\nMeasured Analog: ");
                    printValues(r, g, b);
                    break;
                case 80:
                    measure(2, &r, &g, &b);
                    putsUart0("\nMeasured Analog: ");
                    printValues(r, g, b);
                    break;
                case 81:
                    measure(3, &r, &g, &b);
                    putsUart0("\nMeasured Analog: ");
                    printValues(r, g, b);
                    break;
                case 28:
                    measure(4, &r, &g, &b);
                    putsUart0("\nMeasured Analog: ");
                    printValues(r, g, b);
                    break;
                case 29:
                    measure(5, &r, &g, &b);
                    putsUart0("\nMeasured Analog: ");
                    printValues(r, g, b);
                    break;
                case 73:
                    calc_pH(0);
                    break;
                case 72:
                    calc_pH(1);
                    break;
                case 77:
                    calc_pH(2);
                    break;
                case 76:
                    calc_pH(3);
                    break;
                case 30:
                    calc_pH(4);
                    break;
                case 31:
                    calc_pH(5);
                    break;
                default:
                    break;
            }
        }

        // from Labs 4, 5, 7, and 8
        if (kbhitUart0())
        {
            getsUart0(&data);
            parseFields(&data);

            valid_command = false;

            if(isCommand(&data, "home", 0))
            {
                home();
                valid_command = true;
            }

            if(isCommand(&data, "tube", 1))
            {
                num = getFieldInteger(&data,1);
                if(num <= 5)
                {
                    goTo(num);
                    valid_command = true;
                }
            }

            if(isCommand(&data, "calibrate", 0))
            {
                calibrate();
                valid_command = true;
            }

            if(isCommand(&data, "measure", 2))
            {
                num = getFieldInteger(&data,1);
                if((num >= 1 && num <= 5) && str_cmp(getFieldString(&data,2), "raw"))
                {
                    measure(num, &r, &g, &b);
                    putsUart0("\nMeasured Analog: ");
                    printValues(r, g, b);
                    valid_command = true;
                }
                else if(str_cmp(getFieldString(&data,1), "R") && str_cmp(getFieldString(&data,2), "raw"))
                {
                    measure(0, &r, &g, &b);
                    putsUart0("\nMeasured Analog: ");
                    printValues(r, g, b);
                    valid_command = true;
                }
            }

            if(isCommand(&data, "measure", 2))
            {
                num = getFieldInteger(&data,1);
                if((num >= 1 && num <= 5) && str_cmp(getFieldString(&data,2), "pH"))
                {
                    calc_pH(num);
                    valid_command = true;
                }
                else if(str_cmp(getFieldString(&data,1), "R") && str_cmp(getFieldString(&data,2), "pH"))
                {
                    calc_pH(0);
                    valid_command = true;
                }
            }

            if(!valid_command)
            {
                putsUart0("Invalid command\n\n");
            }
        }
    }
}
