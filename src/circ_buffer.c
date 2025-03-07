#include "circ_buffer.h"

uint32_t CircBuffer_Next(circular_buffer_t* buffer)
{
  if (buffer->index == buffer->size-1) buffer->index = 0;
  return buffer->array[buffer->index++];
}

uint32_t CircBuffer_Prev(circular_buffer_t* buffer)
{
  if (buffer->index == 0) buffer->index = buffer->size-1;
  return buffer->array[buffer->index--];
}