//Ulysses Chaparro 1001718774

#ifndef LAB8_H_
#define LAB8_H_

void printValues(uint16_t r, uint16_t g, uint16_t b);
void calibrate();
void measure(uint8_t tube, uint16_t *r, uint16_t *g, uint16_t *b);
void calc_pH(uint8_t tube);

#endif
