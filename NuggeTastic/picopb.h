#ifndef __PICOPB_H__
#define __PICOPB_H__

#include <Arduino.h>

typedef enum
{
  VARINT = 0,
  I64 = 1,
  STRING = 2,
  I32 = 5,
  INVALID = 7,
} pb_type;

class picopb
{
  public:
    picopb(uint8_t *buffer,size_t length);
    pb_type decode_next(int *id,size_t *size);

    int read_varint(uint64_t *out);
    int read_i64(uint64_t *out);
    int read_string(uint8_t *string,uint8_t bufsz);
    int read_i32(uint32_t *out);

  private:
    uint64_t decode_varint(size_t *size);
    uint8_t *buffer;
    size_t length;
    size_t offset;
    size_t next_size;
    pb_type next_type;
};

#endif //__PICOPB_H__