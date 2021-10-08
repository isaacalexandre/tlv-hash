#ifndef _LIB_TLV_H_
#define _LIB_TLV_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//type definitions
typedef void * ber_tlv_t;

typedef struct {
    uint32_t   tag;
    uint32_t   length;
    uint8_t *  value;
} ber_tlv_object_t;

//constants
#define BER_TLV_CONSTRUCTED_BIT    0x20

ber_tlv_t ber_tlv_create_object(void);
ber_tlv_t ber_tlv_parse_TLV(uint8_t *data, uint32_t dataLength, uint32_t *bytesParsed);

bool ber_tlv_add_data(ber_tlv_t tlv, uint8_t *data, uint32_t dataLength);
bool ber_tlv_add_TLV(ber_tlv_t tlv, ber_tlv_t tlvToAdd);

bool ber_tlv_set_tag(ber_tlv_t tlv, uint32_t tag);
bool ber_tlv_serialize(ber_tlv_t tlv, uint8_t *output, uint32_t *outputLength);

uint32_t ber_tlv_get_tag(ber_tlv_t tlv);
uint32_t ber_tlv_get_length(ber_tlv_t tlv);
uint8_t *ber_tlv_get_value(ber_tlv_t tlv);

void ber_tlv_delete_value(ber_tlv_t tlv);
void ber_tlv_delete_object(ber_tlv_t tlv);

bool ber_tlv_is_constructed(ber_tlv_t tlv);
void ber_tlv_update_value(ber_tlv_t tlv, uint8_t *data, uint32_t data_len);

#ifdef __cplusplus
}
#endif

#endif /* _LIB_TLV_H_ */
