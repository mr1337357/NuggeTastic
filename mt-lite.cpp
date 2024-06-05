#include "mt-lite.h"

#include "mbedtls/aes.h"

static const uint8_t defaultpsk[] = {0xd4, 0xf1, 0xbb, 0x3a, 0x20, 0x29, 0x07, 0x59,
                                     0xf0, 0xbc, 0xff, 0xab, 0xcf, 0x4e, 0x69, 0x01};

mbedtls_aes_context aes;

mt_lite::mt_lite()
{
  mbedtls_aes_init(&aes);
}

void mt_lite::init(uint32_t id)
{
  this->id = id;
  this->sequence = random()<<24 | random() << 16 | random() << 8 | random();
}

void mt_lite::set_aeskey(uint8_t *key,int len)
{
  mbedtls_aes_setkey_enc(&aes, key, len * 8);
}

void mt_lite::send_packet(mt_packet *mtp,int len)
{
  uint8_t *datap = (uint8_t *)mtp;
  LoRa.beginPacket();
  while(len--)
  {
    LoRa.write(*datap++);
  }
  LoRa.endPacket();
}

uint8_t mt_lite::random()
{
  uint8_t byte;
  int i;
  for(i=0;i<7;i++)
  {
    byte<<=1;
    byte|= LoRa.random() & 1;
  }
  return byte;
}

void mt_lite::ack_packet(mt_packet *mtp,int len)
{
  int hopsLeft = mtp->flags & 7;
  if(hopsLeft-- == 0)
  {
    return;
  }
  mtp->flags &= 0xF8;
  mtp->flags |= hopsLeft;
  send_packet(mtp,len);
}

bool mt_lite::seen_seq(uint32_t seq)
{
  int i;
  for(i=0;i<16;i++)
  {
    if(seq == seen_sequence[i])
    {
      return true;
    }
  }
  seen_sequence[last_seen++] = seq;
  last_seen &= 0x0F;
  return false;
}

void mt_lite::fetch_packet(mt_packet *mtp,int size)
{
  uint8_t *dataptr = (uint8_t *)mtp;
  while(size--)
  {
    *dataptr++ = LoRa.read();
  }
}

void mt_lite::update()
{
  size_t pp_size;
  int pp_id;
  int size;
  uint64_t packet_type;
  mt_packet mt;
  size = LoRa.parsePacket();
  if(!size)
  {
    return;
  }
  fetch_packet(&mt,size);
  if(!seen_seq(mt.sequence)) //TODO: ack settings (disable, allow repeats)
  {
    ack_packet(&mt,size);
  }
  if(mt.dest != 0xFFFFFFFF && mt.dest != id)
  {
    return;
  }
  encrypt(&mt,size); //same as decrypt :)
  for(int i = 0;i<size-16;i++)
  {
    Serial.printf("%02X ",mt.payload[i]);
  }
  Serial.println("");
  picopb pp(mt.payload,size-16);
  if(pp.decode_next(&pp_id, &pp_size) != pb_type::VARINT)
  {
    //not somethinge I can handle :(
    return;
  }
  pp.read_varint(&packet_type);
  switch(packet_type)
  {
    case 0x01:
      Serial.println("text");
      break;
    case 0x03:
      Serial.println("position");
      break;
    case 0x04:
      {
        Serial.println("nodeinfo");
        pb_type pt = pp.decode_next(&pp_id,&pp_size);
        Serial.printf("type: %d size %d\n",pt,pp_size);
        break;
      }
    case 0x67:
      Serial.println("telemetry");
      break;
    default:
      Serial.printf("packet type %d\n",packet_type);
      break;
  }
}

void mt_lite::encrypt(mt_packet *packet,int len)
{
  uint8_t scratch[256];
  uint8_t nonce[16];
  uint8_t stream_block[16];
  int offset = 0;
  size_t nc_off = 0;
  
  len-=16;

  memset(&nonce[0],0,16);
  memcpy(&nonce[0],&packet->sequence,4);
  memcpy(&nonce[8],&packet->src,4);
  while(offset < len)
  {
    int numbytes = len - offset;
    if(numbytes > sizeof(scratch))
    {
      numbytes = sizeof(scratch);
    }
    memset(scratch,0,sizeof(scratch));
    memcpy(scratch,&packet->payload[offset],numbytes);
    mbedtls_aes_crypt_ctr(&aes, numbytes, &nc_off, nonce, stream_block, scratch, &packet->payload[offset]);
    offset+=numbytes;
  }
}