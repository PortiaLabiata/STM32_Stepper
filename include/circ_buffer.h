#ifndef CIRC_BUF_H_
#define CIRC_BUF_H_

#include <stdint.h>

/* Types */

typedef struct {
    uint32_t* array;
    uint32_t size;
    uint32_t index;
  } circular_buffer_t;

/* Functions */

uint32_t CircBuffer_Next(circular_buffer_t* buffer);
uint32_t CircBuffer_Prev(circular_buffer_t* buffer);

#endif