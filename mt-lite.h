#include <Arduino.h>

struct _mt_packet
{
    uint32_t dest;
    uint32_t src;
    uint32_t sequence;
    uint8_t flags;
    uint8_t channel;
    uint16_t reserved;
    uint8_t payload[256];
};
typedef struct _mt_packet mt_packet;

class mt_lite
{
  public:
    mt_lite();
    void set_aeskey(uint8_t *key,int len);
    void encrypt(mt_packet *packet,int len);
};