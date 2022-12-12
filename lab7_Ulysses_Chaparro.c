//Ulysses Chaparro 1001718774
//Lab 7 Code

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "wait.h"
#include "lab7.h"

uint8_t phase = 0;
uint8_t position = 0;

//set bitbands for 4 GPIO pins (PE4, PE5, PE3, PE2) BLK, W, Y, G
#define BLK  (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 4*4)))
#define W    (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 5*4)))
#define Y    (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 3*4)))
#define G    (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 2*4)))

#define BLK_MASK 16
#define W_MASK 32
#define Y_MASK 8
#define G_MASK 4

#define DEBUG 0

//sets the global variable position to a known value.
void setPosition(uint8_t pos)
{
    position = pos;
}

//drives the 4 motor controller outputs (via 4 GPIO pins) to the correct voltage level.
void applyPhase(uint8_t phaseIn)
{
    waitMicrosecond(15000);
    switch(phaseIn)
    {
        case 0:
            BLK = 1;
            W = 0;
            Y = 0;
            G = 0;
            break;
        case 1:
            BLK = 0;
            W = 0;
            Y = 1;
            G = 0;
            break;
        case 2:
            BLK = 0;
            W = 1;
            Y = 0;
            G = 0;
            break;
        case 3:
            BLK = 0;
            W = 0;
            Y = 0;
            G = 1;
            break;
    }

    phase = phaseIn;
}

void stepCw()
{
    position ++;
    phase = (phase + 1)%4;
    applyPhase(phase);
}

void stepCcw()
{
    position --;
    phase = (phase - 1)%4;
    if(phase > 3)
    {
        phase = 3;
    }
    applyPhase(phase);
}

void moveTurret(uint8_t tube_pos)
{
    uint8_t i = 0;
    uint8_t V1 = (tube_pos - position);
    uint8_t V2 = (position - tube_pos);
    if(position == 0)
    {
        for(i = 0; i < tube_pos; i++)
        {
            stepCcw();
        }
    }

    else if(position < tube_pos)
    {
        for(i = 0; i < V1; i++)
        {
            stepCcw();
        }
    }

    else if(position > tube_pos)
    {
        for(i = 0; i < V2; i++)
        {
            stepCw();
        }
    }

}

void goTo(uint8_t tube)
{
    switch(tube)
    {
        case 0:
            moveTurret(0);
            position = 0;
            break;
        case 1:
            moveTurret(168);
            position =  168;
            break;
        case 2:
            moveTurret(135);
            position = 135;
            break;
        case 3:
            moveTurret(101);
            position = 101;
            break;
        case 4:
            moveTurret(67);
            position = 67;
            break;
        case 5:
            moveTurret(33);
            position = 33;
            break;
    }

}

void home()
{
    uint8_t i = 0;
    for(i = 0; i < 200; i++)
    {
        stepCw();
    }

    for(i = 0; i < 6; i++)
    {
        stepCcw();
    }

    setPosition(0);
}


