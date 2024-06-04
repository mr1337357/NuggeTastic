#include <Arduino.h>

typedef enum
{
  TEXT_MESSAGE = 1,

} mt_packet_type;

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

struct _text_message
{
  uint64_t mt_packet_type;
  uint8_t text_message[256];
};
typedef struct _text_message text_message;



class mt_lite
{
  public:
    mt_lite();
    void set_aeskey(uint8_t *key,int len);
    void encrypt(mt_packet *packet,int len);
};