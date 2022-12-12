//Ulysses Chaparro 1001718774

// Jason Losh origin code
//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Green LED:
// PF3 drives an NPN transistor that powers the green LED
// PD0 is GPI pin

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "lab6.h"

uint32_t time[50];
uint8_t count = 0; //index of edges (count 0 is 0th edge)
uint8_t code;
bool valid = false;
bool valid_comp = true;

uint8_t address[8];
uint8_t inv_address[8];
uint8_t data[8];
uint8_t inv_data[8];

char str[10]; //for uart0 text formatting

uint32_t time_32bits[33];
char this_keyName[20];

char keyNames[44][350] = {"Brightness Up", "Brightness Down", "Play", "Power", "Red", "Green", "Blue",
                         "White", "Row 3 Button 1", "Row 3 Button 2", "Row 3 Button 3", "Row 3 Button 4",
                         "Row 4 Button 1", "Row 4 Button 2", "Row 4 Button 3", "Row 4 Button 4",
                         "Row 5 Button 1", "Row 5 Button 2", "Row 5 Button 3", "Row 5 Button 4",
                         "Row 6 Button 1", "Row 6 Button 2", "Row 6 Button 3", "Row 6 Button 4",
                         "Red Up", "Green Up", "Blue Up", "QUICK", "Red Down", "Green Down", "Blue Down",
                         "SLOW", "DIY1", "DIY2", "DIY3", "AUTO", "DIY4", "DIY5", "DIY6", "FLASH", "JUMP3",
                         "JUMP7", "FADE3", "FADE7"};

// Pin bitbands
#define GREEN_LED    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4)))

// PortD masks (GPI pin PD0)
#define FREQ_IN_MASK 1
// PortF masks
#define GREEN_LED_MASK 8

void enableTimerMode()
{
    WTIMER2_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off counter before reconfiguring
    WTIMER2_CFG_R = 4;                               // configure as 32-bit counter (A only)
    WTIMER2_TAMR_R = TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP | TIMER_TAMR_TACDIR; // configure for edge time mode, count up
    WTIMER2_CTL_R = TIMER_CTL_TAEVENT_NEG;           // measure time from positive edge to negative edge
    WTIMER2_IMR_R = TIMER_IMR_CAEIM;                 // turn-on interrupts
    WTIMER2_TAV_R = 0;                               // zero counter for first period
    WTIMER2_CTL_R |= TIMER_CTL_TAEN;                 // turn-on counter
    NVIC_EN3_R |= 1 << (INT_WTIMER2A-16-96);         // turn-on interrupt 112 (WTIMER2A) //NOT SURE ABOUT THIS LINE
}

uint8_t bits_to_integer(uint8_t bits[8])
{
    uint8_t i;
    uint8_t two_power = 1; //two_power of 0 equals 1
    uint8_t integer = 0;
    for(i = 0; i < 8; i++)
    {
        integer += bits[7 - i] * two_power;
        two_power *= 2;
    }

    return integer;
}

void print_key()
{
    //integer values of binary data for each key (flipped)
    uint8_t keys[] = {92,93,65,64,88,89,69,68,84,85,73,72,80,81,77,76,28,29,30,31,24,
                      25,26,27,20,21,22,23,16,17,18,19,12,13,14,15,8,9,10,11,4,5,6,7};

    //verify that address is 0 (unique to remote)
    if((bits_to_integer(address) == 0))
    {
        uint8_t i;
        for(i = 0; i < 44; i++)
        {
            if(code == keys[i])
            {
                putsUart0("Key: ");
                putsUart0(keyNames[i]); //prints key name to uart0
                putcUart0(' ');
                putsUart0("Code: ");
                sprintf(str, "%d", code);
                putsUart0(str);
                putsUart0("\n");
                return;
            }
        }
    }
    else
    {
        valid = false; //address is not 0
    }
}

void validateBits()
{
    int i;
    for(i = 1; i < 34; i++)
    {
        time_32bits[i-1] = time[i]; //time_32bits[] is still unflipped just like time[]
    }

    //setting address
    for(i = 0; i < 8; i++)
    {
        //if 1.5T < t < 2.5T , '0'
        if(time_32bits[i+1] - time_32bits[i] >= 33750 &&
           time_32bits[i+1] - time_32bits[i] <= 56250)
        {
            address[7-i] = 0;
        }

        //if 3.5T < t < 4.5T , '1'
        if(time_32bits[i+1] - time_32bits[i] >= 78750 &&
           time_32bits[i+1] - time_32bits[i] <= 101250)
        {
            address[7-i] = 1;
        }
    }

    //setting inv_address
    for(i = 8; i < 16; i++)
    {
        //if 1.5T < t < 2.5T , '0'
        if(time_32bits[i+1] - time_32bits[i] >= 33750 &&
           time_32bits[i+1] - time_32bits[i] <= 56250)
        {
            inv_address[15-i] = 0;
        }

        //if 3.5T < t < 4.5T , '1'
        if(time_32bits[i+1] - time_32bits[i] >= 78750 &&
           time_32bits[i+1] - time_32bits[i] <= 101250)
        {
            inv_address[15-i] = 1;
        }
    }

    //setting data
    for(i = 16; i < 24; i++)
    {
        //if 1.5T < t < 2.5T , '0'
        if(time_32bits[i+1] - time_32bits[i] >= 33750 &&
           time_32bits[i+1] - time_32bits[i] <= 56250)
        {
            data[23-i] = 0;
        }

        //if 3.5T < t < 4.5T , '1'
        if(time_32bits[i+1] - time_32bits[i] >= 78750 &&
           time_32bits[i+1] - time_32bits[i] <= 101250)
        {
            data[23-i] = 1;
        }
    }

    //setting inv_data
    for(i = 24; i < 32; i++)
    {
        //if 1.5T < t < 2.5T , '0'
        if(time_32bits[i+1] - time_32bits[i] >= 33750 &&
           time_32bits[i+1] - time_32bits[i] <= 56250)
        {
            inv_data[31-i] = 0;
        }

        //if 3.5T < t < 4.5T , '1'
        if(time_32bits[i+1] - time_32bits[i] >= 78750 &&
           time_32bits[i+1] - time_32bits[i] <= 101250)
        {
            inv_data[31-i] = 1;
        }
    }

    //check if complementary
    for(i = 0; i < 8; i++)
    {
        if(address[i] == 0) //then inv_address[i] should be 1
        {
            if(inv_address[i] != 1)
            {
                valid_comp = false;
            }
        }

        if(address[i] == 1) //then inv_address[i] should be 0
        {
            if(inv_address[i] != 0)
            {
                valid_comp = false;
            }
        }

        if(data[i] == 0) //then inv_data[i] should be 1
        {
            if(inv_data[i] != 1)
            {
                valid_comp = false;
            }
        }

        if(data[i] == 1) //then inv_data[i] should be 1
        {
            if(inv_data[i] != 0)
            {
                valid_comp = false;
            }
        }
    }

    if(valid_comp)
    {
        valid = true;
    }

    if(valid) //valid was set to false globally, so it should remain false unless invalid bits set it to true
    {
        code = bits_to_integer(data);
    }
}

void gpioIsr()
{
    if (WTIMER2_TAV_R > 80*40000)
    {
        count = 0;
    }

    time[count] = WTIMER2_TAV_R;

    if(count == 0)
    {
        WTIMER2_TAV_R = 0;
        time[count] = 0;
        count++;
    }

    else
    {
        time[count] = WTIMER2_TAR_R;
        if(count == 1)
        {
            if(time[1] - time[0] >= (520000) && time[1] - time[0] <= (560000))
            {
                count++;
            }
            else
            {
                count = 0;
            }
        }

        else
        {
            if(count > 1)
            {
                if((time[count] - time[count-1] >= 33750 && time[count] - time[count-1] <= 56250) ||
                   (time[count] - time[count-1] >= 78750 && time[count] - time[count-1] <= 101250))
                {
                    count++;
                }
                else
                {
                    count = 0;
                }
            }
        }
    }

    if(count > 33) //array has reached last edge 34 might have to change to 34
    {
        count = 0;
        validateBits();
    }

     WTIMER2_ICR_R = TIMER_ICR_CAECINT; //clear the interrupt flag
}

