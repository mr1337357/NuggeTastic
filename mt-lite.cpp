#include "mt-lite.h"

#include "mbedtls/aes.h"

static const uint8_t defaultpsk[] = {0xd4, 0xf1, 0xbb, 0x3a, 0x20, 0x29, 0x07, 0x59,
                                     0xf0, 0xbc, 0xff, 0xab, 0xcf, 0x4e, 0x69, 0x01};


mbedtls_aes_context aes;

mt_lite::mt_lite()
{
  mbedtls_aes_init(&aes);
}

void mt_lite::set_aeskey(uint8_t *key,int len)
{
  mbedtls_aes_setkey_enc(&aes, key, len * 8);
}

void mt_lite::encrypt(mt_packet *packet,int len)
{
  uint8_t scratch[16];
  uint8_t nonce[16];
  uint8_t stream_block[16];
  int offset = 0;
  size_t nc_off = 0;
  
  len-=16;

  memcpy(nonce,&packet->sequence,4);
  memset(nonce+4,0,4);
  memcpy(nonce+8,&packet->src,4);
  while(offset < len)
  {
    int numbytes = len - offset;
    if(numbytes > 16)
    {
      numbytes = 16;
    }
    memset(scratch,0,16);
    memcpy(scratch,&packet->payload[offset],numbytes);
    mbedtls_aes_crypt_ctr(&aes, numbytes, &nc_off, nonce, stream_block, scratch, &packet->payload[offset]);
    offset+=numbytes;
  }
}