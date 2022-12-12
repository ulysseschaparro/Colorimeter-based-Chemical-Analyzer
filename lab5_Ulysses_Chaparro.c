//Ulysses Chaparro 1001718774

// Serial Example
// Jason Losh origin code

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Red LED:
//   PF1 drives an NPN transistor that powers the red LED
// Green LED:
//   PF3 drives an NPN transistor that powers the green LED
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port
//   Configured to 115,200 baud, 8N1

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "lab5.h"


//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void parseFields(USER_DATA *data)
{
    bool delimiter = true;
    data->fieldCount = 0;
    uint8_t i = 0; //i is position in buffer (offset)
    uint8_t j = 0;

    //clearing arrays
    for(j = 0; j < MAX_FIELDS; j++)
    {
        data->fieldPosition[j] = MAX_CHARS + 1;
        data->fieldType[j] = '\0';
    }

    while(data->buffer[i] != '\0') //loop until buffer ends
    {
        if((data->buffer[i] >= 65 && data->buffer[i] <= 90) || (data->buffer[i] >= 97 && data->buffer[i] <= 122)) //if char is Alpha
        {
            if(delimiter == true) //if transition
            {
                data->fieldType[data->fieldCount] = 'a';
                data->fieldPosition[data->fieldCount++] = i;

                if(data->fieldCount == MAX_FIELDS)
                {

                    return;
                }
                else
                {
                    delimiter = false;
                }
            }
        }
        else if(data->buffer[i] >= 48 && data->buffer[i] <= 57) //if char is Num
        {
            if(delimiter == true) //if transition
            {
                data->fieldType[data->fieldCount] = 'n';
                data->fieldPosition[data->fieldCount++] = i;

                if(data->fieldCount == MAX_FIELDS)
                {
                    return;
                }

                else
                {
                    delimiter = false;
                }
            }
        }
        else //else, char is delimiter
        {
            delimiter = true;
            data->buffer[i] = '\0';
        }

        i++;
    }

    return;
}

char* getFieldString(USER_DATA *data, uint8_t fieldNumber)
{
    if(fieldNumber <= data->fieldCount)
    {
        uint8_t offset = data->fieldPosition[fieldNumber];
        char field_string[MAX_CHARS + 1];

        if(offset == MAX_CHARS + 1)
        {
            return NULL; //there is no field at fieldNumber
        }
        else
        {
            uint8_t i = 0;
            while(data->buffer[offset] != '\0') //go through the field and transfer characters to field_string
            {
                field_string[i] = data->buffer[offset];
                offset++;
                i++;
            }
            field_string[i] = '\0'; //set last char in string to null
            return field_string;
        }
    }
    else
    {
        return NULL;
    }

}

int32_t getFieldInteger(USER_DATA *data, uint8_t fieldNumber)
{
    if(data->fieldType[fieldNumber] == 'n')
    {
        uint8_t offset = data->fieldPosition[fieldNumber];
        int32_t field_integer = 0;
        while(data->buffer[offset] != '\0') //go through the field and transfer characters to field_integer
        {
            field_integer = (field_integer * 10) + (data->buffer[offset] - '0'); //conversion from each char to a complete integer
            offset++;
        }
        return field_integer;
    }
    else
    {
        return 0;
    }
}

bool str_cmp(char *string1, char *string2)
{
    uint8_t i = 0;
    while(string1[i] != '\0' || string2[i] != '\0')
    {
        if(string1[i] != string2[i])
        {
            return false;
        }
        i++;
    }
    return true;
}

bool isCommand(USER_DATA *data, const char strCommand[], uint8_t minArguments)
{
    if((data->fieldCount) >= minArguments)
    {
       return str_cmp(data->buffer, strCommand);
    }
    else
    {
        return false; //number of arguments is not >= minArguments
    }
}


