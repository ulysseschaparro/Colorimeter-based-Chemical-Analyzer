//Ulysses Chaparro 1001718774

#ifndef LAB4_H_
#define LAB4_H_

#define MAX_CHARS 80
#define MAX_FIELDS 5
typedef struct USER_DATA
        {
            char buffer[MAX_CHARS+1];
            uint8_t fieldCount;
            uint8_t fieldPosition[MAX_FIELDS];
            char fieldType[MAX_FIELDS];
        } USER_DATA;

void getsUart0(USER_DATA *data);

#endif
