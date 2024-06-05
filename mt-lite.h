#ifndef __MT_LITE_H__
#define __MT_LITE_H__
#include <Arduino.h>
#include <LoRa.h>

#include "picopb.h"

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

struct _user
{
  uint8_t id[32];
  uint8_t longname[32];
  uint8_t shortname[4];
}


class mt_lite
{
  public:
    mt_lite();
    void init(uint32_t id);
    void update();
    void set_aeskey(uint8_t *key,int len);
    void encrypt(mt_packet *packet,int len);
    uint8_t random();
  private:
    void send_packet(mt_packet *mtp,int len);
    void ack_packet(mt_packet *mtp,int len);
    void fetch_packet(mt_packet *mtp,int size);
    bool seen_seq(uint32_t seq);
    uint32_t id;
    uint32_t sequence;
    //LoRaClass &lora;
    uint32_t seen_sequence[16];
    uint8_t last_seen;
};

#endif //__MT_LITE_H__