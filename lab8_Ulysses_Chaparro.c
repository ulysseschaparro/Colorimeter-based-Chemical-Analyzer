//Ulysses Chaparro Lab 8 Code

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "wait.h"
#include "rgb_led.h"
#include "adc0.h"
#include "lab7.h"
#include "lab8.h"

#define ANALOG_MASK 32
#define BLK_MASK 16
#define W_MASK 32
#define Y_MASK 8
#define G_MASK 4

uint16_t pwm_r, pwm_g, pwm_b;
uint16_t r, g, b;
uint16_t raw;
uint16_t dc;
float ref1, ref2;
float pH;

//6.8 pH
uint16_t ref1_r = 3400;
uint16_t ref1_g = 3100;
uint16_t ref1_b = 1140;

//7.2 pH
uint16_t ref2_r = 3900;
uint16_t ref2_g = 3700;
uint16_t ref2_b = 1700;

//7.5 pH
uint16_t ref3_r = 2660;
uint16_t ref3_g = 1360;
uint16_t ref3_b = 900;

//7.8 pH
uint16_t ref4_r = 3030;
uint16_t ref4_g = 1060;
uint16_t ref4_b = 850;

//8.2 pH
uint16_t ref5_r = 2700;
uint16_t ref5_g = 520;
uint16_t ref5_b = 1060;

float distance[5];
uint16_t dist_closest, dist_2ndclosest;

char str1[20], str2[20], str3[20];

void printValues(uint16_t r, uint16_t g, uint16_t b)
{
    sprintf(str1, "%d", r);
    sprintf(str2, "%d", g);
    sprintf(str3, "%d", b);

    putsUart0("("); putsUart0(str1); putsUart0(", ");
                    putsUart0(str2); putsUart0(", ");
                    putsUart0(str3);
    putsUart0(")\n");
}

void calibrate()
{
    home();

    //red
    raw = 0;
    for(dc = 0; raw < 3072; dc = (dc+1)%1024)
    {
        setRgbColor(dc, 0, 0);
        waitMicrosecond(10000);
        raw = readAdc0Ss3();
    }
    r = raw;
    pwm_r = dc;

    //green
    raw = 0;
    for(dc = 0; raw < 3072; dc = (dc+1)%1024)
    {
        setRgbColor(0, dc, 0);
        waitMicrosecond(10000);
        raw = readAdc0Ss3();
    }
    g = raw;
    pwm_g = dc;

    //blue
    raw = 0;
    for(dc = 0; raw < 3072; dc = (dc+1)%1024)
    {
        setRgbColor(0, 0, dc);
        waitMicrosecond(10000);
        raw = readAdc0Ss3();
    }
    b = raw;
    pwm_b = dc;

    setRgbColor(0,0,0);

    putsUart0("\nCalibrated Analog: ");
    printValues(r, g, b);
    putsUart0("PWM: ");
    printValues(pwm_r, pwm_g, pwm_b);
}

void measure(uint8_t tube, uint16_t *r, uint16_t *g, uint16_t *b)
{
    goTo(tube);
    waitMicrosecond(400000);

    //red
    setRgbColor(pwm_r, 0, 0);
    waitMicrosecond(300000);
    *r = readAdc0Ss3();

    //green
    setRgbColor(0, pwm_g, 0);
    waitMicrosecond(300000);
    *g = readAdc0Ss3();

    //blue
    setRgbColor(0, 0, pwm_b);
    waitMicrosecond(300000);
    *b = readAdc0Ss3();

    setRgbColor(0,0,0);
}

void calc_pH(uint8_t tube)
{
    measure(tube, &r, &g, &b);

    uint8_t i, j, k;

    distance[0]= (r - ref1_r)*(r - ref1_r) + (g - ref1_g)*(g - ref1_g) + (b - ref1_b)*(b - ref1_b);
    distance[1] = (r - ref2_r)*(r - ref2_r) + (g - ref2_g)*(g - ref2_g) + (b - ref2_b)*(b - ref2_b);
    distance[2] = (r - ref3_r)*(r - ref3_r) + (g - ref3_g)*(g - ref3_g) + (b - ref3_b)*(b - ref3_b);
    distance[3] = (r - ref4_r)*(r - ref4_r) + (g - ref4_g)*(g - ref4_g) + (b - ref4_b)*(b - ref4_b);
    distance[4] = (r - ref5_r)*(r - ref5_r) + (g - ref5_g)*(g - ref5_g) + (b - ref5_b)*(b - ref5_b);

    dist_closest = distance[0];
    j = 0;
    dist_2ndclosest = distance[1];
    k = 1;

    if (dist_2ndclosest < dist_closest)
    {
        dist_closest = distance[1];
        j = 1;
        dist_2ndclosest = distance[0];
        k = 0;
    }

    for (i = 2; i < 5; i++)
    {
        if (distance[i] < dist_closest)
        {
            dist_2ndclosest = dist_closest;
            k = j;
            dist_closest = distance[i];
            j = i;
        }
        else if (distance[i] < dist_2ndclosest)
        {
            dist_2ndclosest = distance[i];
            k = i;
        }
    }

    switch(j)
    {
        case 0:
            ref1 = 6.8;
            break;
        case 1:
            ref1 = 7.2;
            break;
        case 2:
            ref1 = 7.5;
            break;
        case 3:
            ref1 = 7.8;
            break;
        case 4:
            ref1 = 8.2;
            break;
    }

    switch(k)
    {
        case 0:
            ref2 = 6.8;
            break;
        case 1:
            ref2 = 7.2;
            break;
        case 2:
            ref2 = 7.5;
            break;
        case 3:
            ref2 = 7.8;
            break;
        case 4:
            ref2 = 8.2;
            break;
    }

    pH = (fabs(ref2 - ref1) * (distance[j]/(distance[j]+distance[k]))) + ref1;

    sprintf(str1, "%.4f", pH);
    putsUart0("pH: ");
    putsUart0(str1);
    putsUart0("\n");
}

