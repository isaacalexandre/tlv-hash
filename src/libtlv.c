#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "llist.h"
#include "libtlv.h"
#include "util.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define ISOLATE_BYTE3(x) ((uint8_t) ((x & 0xFF000000) >> 24))
#define ISOLATE_BYTE2(x) ((uint8_t) ((x & 0x00FF0000) >> 16))
#define ISOLATE_BYTE1(x) ((uint8_t) ((x & 0x0000FF00) >> 8))
#define ISOLATE_BYTE0(x) ((uint8_t)  (x & 0x000000FF))
#define UNUSED(x) (void)(x)
/******************************************************
 *                    Constants
 ******************************************************/
#define BER_TLV_CONSTRUCTED_BIT    0x20
/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef void * ber_tlv_t;

typedef struct {
    uint32_t   tag;
    uint32_t   length;
    uint8_t *  value;
} ber_tlv_object_t;

typedef struct {
    uint32_t   tag;
    uint16_t   num;//Number of children
    uint32_t   child_tag[STRUCT_NUM_CHILD];
} ber_tlv_constructed_object_t;
/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/
llist *tags_primitive; 
llist *tags_constructed; 
bool b_debug_enabled = false;
/******************************************************
 *               Function Definitions
 ******************************************************/
ber_tlv_t ber_tlv_create_object(void);
ber_tlv_t ber_tlv_parse_TLV(uint8_t *data, uint32_t dataLength);

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
/******************************************************
 *               Interface functions
 ******************************************************/

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
uint32_t ber_tlv_init(bool b_debug)
{
    //Initialize thes lists and the debug
    tags_primitive = llist_create(NULL);
    tags_constructed = llist_create(NULL); 
    b_debug_enabled = b_debug;
    if(tags_constructed == NULL || tags_primitive == NULL)
        return -1;
    else
        return 0;     
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
uint32_t ber_tlv_terminate(void)
{
    return 0;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
uint32_t ber_tlv_set(uint8_t * p_value, int32_t i32_size)
{    
    //Allocate in memory the object
    ber_tlv_t* ptlv = ber_tlv_parse_TLV(p_value, i32_size);   
    ber_tlv_object_t* ptlv_obj = (ber_tlv_object_t*)ptlv;  

    if(ptlv == NULL)
        goto error;

    if(b_debug_enabled) {
        printf("TAG:%02X LEN:%d DATA: ",ptlv_obj->tag ,ptlv_obj->length);
        for(uint32_t i=0; i < ptlv_obj->length; i++)
            printf("%02X ",ptlv_obj->value[i]);
    }

    if(ber_tlv_is_constructed((ber_tlv_t*)ptlv)) {
        printf("TESTE");
    }

    return 0;

    error:
    ber_tlv_delete_object((ber_tlv_t*)ptlv);
    return -1;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
uint32_t ber_tlv_get(uint8_t * resp_str, int32_t *i32_resp_size, uint8_t * p_value, int32_t i32_size)
{
    UNUSED(resp_str);
    UNUSED(i32_resp_size);
    UNUSED(p_value);
    UNUSED(i32_size);
    return 0;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
ber_tlv_t ber_tlv_create_object(void)
{
    ber_tlv_object_t *ptr = NULL;

    //allocate memory
    ptr = (ber_tlv_object_t *) malloc (sizeof (ber_tlv_object_t));
    if (!ptr) {
        return NULL;
    }

    //initialize
    ptr->length = 0;
    ptr->tag = 0;
    ptr->value = NULL;

    //done
    return ptr;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
bool ber_tlv_add_data(ber_tlv_t tlv, uint8_t *data, uint32_t dataLength)
{
    ber_tlv_object_t *ptrTLV = NULL;
    uint32_t newLength = 0;
    uint8_t *newData = NULL;

    //sanity checks
    if (!tlv) {
        return false;
    }
    if (!data) {
        return false;
    }
    if (dataLength < 1) {
        return false;
    }

    //grab pointer
    ptrTLV = (ber_tlv_object_t *) tlv;

    //evaluate new length
    newLength = ptrTLV->length + dataLength;

    //allocate more memory
    newData = (uint8_t *) malloc (newLength + 1); //this +1 is to facilitate debugging

    if (!newData) {
        return false;                                   //original TLV has not been changed

    }
    memset(newData, 0, newLength + 1); //this +1 is for character null

    //copy old data
    if (ptrTLV->length > 0) {
        memcpy(newData, ptrTLV->value, ptrTLV->length);
    }

    //copy new data
    memcpy(&(newData[ptrTLV->length]), data, dataLength);

    //free old data
    free(ptrTLV->value);

    //adjust pointer
    ptrTLV->value = newData;
    ptrTLV->length = newLength;

    //done
    return true;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
bool ber_tlv_add_TLV(ber_tlv_t tlv, ber_tlv_t tlvToAdd)
{
    ber_tlv_object_t *ptrTLV = NULL;

    uint32_t serializedLength = 0;
    uint32_t newLength = 0;
    uint8_t *newData = NULL;

    //sanity checks
    if (!tlv) {
        return false;
    }
    if (!tlvToAdd) {
        return false;
    }

    //grab pointer
    ptrTLV = (ber_tlv_object_t *) tlv;

    //lets check how much room the other TLV requires
    if (!ber_tlv_serialize (tlvToAdd, NULL, &serializedLength)) {
        return false;
    }

    //evaluate new length
    newLength = ptrTLV->length + serializedLength;

    //allocate more memory
    newData = (uint8_t *) malloc (newLength + 1); //this +1 is to add a character null to facilitate debugging

    if (!newData) {
        return false;                                   //original TLV has not been changed

    }
    memset(newData, 0, newLength + 1); //this +1 is for character null

    //copy old data
    if (ptrTLV->length > 0) {
        memcpy(newData, ptrTLV->value, ptrTLV->length);
    }

    //serialize TLV
    if (!ber_tlv_serialize (tlvToAdd, &(newData[ptrTLV->length]), &serializedLength)) {
        free (newData);
        return false;
    }

    //free old data
    free (ptrTLV->value);

    //adjust pointer
    ptrTLV->value = newData;
    ptrTLV->length = newLength;

    //done
    return true;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
void ber_tlv_update_value(ber_tlv_t tlv, uint8_t *data, uint32_t data_len)
{
    ber_tlv_object_t *ptr = NULL;
    uint32_t copy_len = 0;

    //sanity check
    if (!tlv || !data || !data_len) {
        return;
    }

    //grab pointer
    ptr = (ber_tlv_object_t *) tlv;

    if (data_len > ptr->length)
        copy_len = ptr->length;
    else
        copy_len = data_len;

    memcpy(ptr->value, data, copy_len);

    //done
    return;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
void ber_tlv_delete_value(ber_tlv_t tlv)
{
    ber_tlv_object_t *ptr = NULL;

    //sanity check
    if (!tlv) {
        return;
    }

    //grab pointer
    ptr = (ber_tlv_object_t *) tlv;

    //free memory
    if (ptr->value) {
        free(ptr->value);
        ptr->length = 0;
    }

    //done
    return;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
void ber_tlv_delete_object(ber_tlv_t tlv)
{
    //sanity check
    if (!tlv) {
        return;
    }

    //delete value
    ber_tlv_delete_value(tlv);

    //free memory
    free(tlv);

    //done
    return;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
uint32_t ber_tlv_get_tag(ber_tlv_t tlv)
{
    ber_tlv_object_t *ptr = NULL;

    //sanity check
    if (!tlv) {
        return 0;
    }

    //grab pointer
    ptr = (ber_tlv_object_t *) tlv;

    //return the tag -- turn OFF the constructed bit
    //return (ptr->tag & (~BER_TLV_CONSTRUCTED_BIT));

    //don't turn off the constructed bit!
    return ptr->tag;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
bool ber_tlv_set_tag(ber_tlv_t tlv, uint32_t tag)
{
    const uint8_t CHECK_MSB = 0x80;
    const uint8_t CHECK_PATTERN = 0x1F;
    const uint8_t CHECK_ZERO = 0x00;
    ber_tlv_object_t *ptr = NULL;
    uint8_t buffer[4];

    //sanity check
    if (!tlv) {
        return false;
    }

    //grab pointer
    ptr = (ber_tlv_object_t *) tlv;

    //sanity check on tag value - must be valid
    buffer[0] = ISOLATE_BYTE3(tag);
    buffer[1] = ISOLATE_BYTE2(tag);
    buffer[2] = ISOLATE_BYTE1(tag);
    buffer[3] = ISOLATE_BYTE0(tag);

    if (buffer[0] != CHECK_ZERO) {
        if ((buffer[0] & CHECK_PATTERN) != CHECK_PATTERN) {
            return false;                         //first byte must be xxx11111
        }
        if ((buffer[1] & CHECK_MSB) != CHECK_MSB) {
            return false;                         //second byte must be 1xxxxxxx
        }
        if ((buffer[2] & CHECK_MSB) != CHECK_MSB) {
            return false;                         //third byte must be 1xxxxxxx
        }
        if ((buffer[3] & CHECK_MSB) != CHECK_ZERO) {
            return false;                         //last byte must be 0xxxxxxx
        }
    } else if (buffer[1] != CHECK_ZERO) {
        if ((buffer[1] & CHECK_PATTERN) != CHECK_PATTERN) {
            return false;                         //first byte must be xxx11111
        }
        if ((buffer[2] & CHECK_MSB) != CHECK_MSB) {
            return false;                         //second byte must be 1xxxxxxx
        }
        if ((buffer[3] & CHECK_MSB) != CHECK_ZERO) {
            return false;                         //last byte must be 0xxxxxxx
        }
    } else if (buffer[2] != CHECK_ZERO) {
        if ((buffer[2] & CHECK_PATTERN) != CHECK_PATTERN) {
            return false;                         //first byte must be xxx11111
        }
        if ((buffer[3] & CHECK_MSB) != CHECK_ZERO) {
            return false;                         //last byte must be 0xxxxxxx
        }
    } else
    if ((buffer[3] & CHECK_PATTERN) == CHECK_PATTERN) {
        return false;                             //first byte cannot be xxx11111


    }
    //set tag
    ptr->tag = tag;

    //done
    return true;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
bool ber_tlv_serialize(ber_tlv_t tlv, uint8_t *output, uint32_t *outputLength)
{
    ber_tlv_object_t *ptr = NULL;
    uint32_t bytesNeeded = 0;
    uint8_t buffer[4];
    uint32_t offset = 0;

    //sanity check
    if (!tlv) {
        return false;
    }
    if (!outputLength) {
        return false;
    }

    //grab pointer
    ptr = (ber_tlv_object_t *) tlv;

    //lets count how many bytes we need
    //how many bytes for the tag?
    if ((ptr->tag & 0xFF000000) != 0) {
        bytesNeeded += 4;
    } else if ((ptr->tag & 0x00FF0000) != 0) {
        bytesNeeded += 3;
    } else if ((ptr->tag & 0x0000FF00) != 0) {
        bytesNeeded += 2;
    } else {
        bytesNeeded += 1; //tag takes at least 1 byte
    }
    //how many bytes for the length?
    if (ptr->length <= 127) {
        bytesNeeded += 1;
    } else if (ptr->length <= 0xFF) {
        bytesNeeded += 2;
    } else if (ptr->length <= 0xFFFF) {
        bytesNeeded += 3;
    } else if (ptr->length <= 0xFFFFFF) {
        bytesNeeded += 4;
    } else {
        bytesNeeded += 5;
    }

    //how many bytes for the value?
    bytesNeeded += ptr->length;

    //check if caller just wanted to know the length
    if (!output) {
        *outputLength = bytesNeeded;
        return true;
    }

    //check if we have enough room
    if (bytesNeeded > (*outputLength)) {
        return false;
    }

    //serialize BER-TLV

    //serialize tag
    buffer[0] = ISOLATE_BYTE3(ptr->tag);
    buffer[1] = ISOLATE_BYTE2(ptr->tag);
    buffer[2] = ISOLATE_BYTE1(ptr->tag);
    buffer[3] = ISOLATE_BYTE0(ptr->tag);

    if (buffer[0] != 0) {
        output[offset++] = buffer[0];
        output[offset++] = buffer[1];
        output[offset++] = buffer[2];
        output[offset++] = buffer[3];
    } else if (buffer[1] != 0) {
        output[offset++] = buffer[1];
        output[offset++] = buffer[2];
        output[offset++] = buffer[3];
    } else if (buffer[2] != 0) {
        output[offset++] = buffer[2];
        output[offset++] = buffer[3];
    } else {
        output[offset++] = buffer[3];
    }

    //serialize length
    if (ptr->length <= 127) {
        output[offset++] = ISOLATE_BYTE0(ptr->length);
    } else if (ptr->length <= 0xFF) {
        output[offset++] = 0x81; //1 bytes

        output[offset++] = ISOLATE_BYTE0(ptr->length);
    } else if (ptr->length <= 0xFFFF) {
        output[offset++] = 0x82; //2 bytes

        output[offset++] = ISOLATE_BYTE1(ptr->length);
        output[offset++] = ISOLATE_BYTE0(ptr->length);
    } else if (ptr->length <= 0xFFFFFF) {
        output[offset++] = 0x83; //3 bytes

        output[offset++] = ISOLATE_BYTE2(ptr->length);
        output[offset++] = ISOLATE_BYTE1(ptr->length);
        output[offset++] = ISOLATE_BYTE0(ptr->length);
    } else {
        output[offset++] = 0x84; //4 bytes

        output[offset++] = ISOLATE_BYTE3(ptr->length);
        output[offset++] = ISOLATE_BYTE2(ptr->length);
        output[offset++] = ISOLATE_BYTE1(ptr->length);
        output[offset++] = ISOLATE_BYTE0(ptr->length);


    }

    //serialize value
    memcpy (&(output[offset]), ptr->value, ptr->length);

    //update length
    *outputLength = bytesNeeded;

    //done
    return true;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
uint32_t ber_tlv_get_length(ber_tlv_t tlv)
{
    ber_tlv_object_t *ptr = NULL;

    //sanity check
    if (!tlv) {
        return 0;
    }

    //grab pointer
    ptr = (ber_tlv_object_t *) tlv;

    //return the length
    return ptr->length;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
uint8_t *ber_tlv_get_value(ber_tlv_t tlv)
{
    ber_tlv_object_t *ptr = NULL;

    //sanity check
    if (!tlv) {
        return NULL;
    }

    //grab pointer
    ptr = (ber_tlv_object_t *) tlv;

    //return the value
    return ptr->value;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
ber_tlv_t ber_tlv_parse_TLV(uint8_t *data, uint32_t dataLength)
{
    ber_tlv_object_t *ptr = NULL;
    uint32_t bytesLeft;
    uint32_t offset;
    uint32_t tag;
    uint32_t length;
    uint32_t tagBytes;


    //sanity checks
    if (!data) {
        return NULL;
    }

    //start parsing
    tagBytes = 1; //our tag uses at least 1 byte
    bytesLeft = dataLength;
    tag = 0;
    length = 0;
    offset = 0;

    //parse tag - byte 1
    //are there bytes left?
    if (bytesLeft < 1) {
        return NULL;
    }

    //update tag value
    tag += (uint32_t) data[offset];

    //check if more tag bytes follow
    if ((data[offset] & 0x1F) == 0x1F) {
        tagBytes++;
    }

    //we used 1 byte
    offset += 1;
    bytesLeft -= 1;

    //parse tag - byte 2
    if (tagBytes >= 2) {
        //are there bytes left?
        if (bytesLeft < 1) {
            return NULL;
        }

        //update tag value
        tag *= 256;
        tag += (uint32_t) data[offset];

        //check if more tag bytes follow
        if ((data[offset] & 0x80) == 0x80) {
            tagBytes++;                            //there's one more tagByte following

        }
        //we used 1 byte
        offset += 1;
        bytesLeft -= 1;
    }

    //parse tag - byte 3
    if (tagBytes >= 3) {
        //are there bytes left?
        if (bytesLeft < 1) {
            return NULL;
        }

        //update tag value
        tag *= 256;
        tag += (uint32_t) data[offset];

        //check if more tag bytes follow
        if ((data[offset] & 0x80) == 0x80) {
            tagBytes++;     //there's one more tagByte following

        }
        //we used 1 byte
        offset += 1;
        bytesLeft -= 1;
    }

    //parse tag - byte 4
    if (tagBytes >= 4) {
        //are there bytes left?
        if (bytesLeft < 1) {
            return NULL;
        }

        //update tag value
        tag *= 256;
        tag += (uint32_t) data[offset];

        //check if more tag bytes follow
        if ((data[offset] & 0x80) == 0x80) {
            return NULL; //we only support tags up to 4 bytes long

        }
        //we used 1 byte
        offset += 1;
        bytesLeft -= 1;
    }

    //parse length
    //are there bytes left?
    if (bytesLeft < 1) {
        return NULL;
    }

    //check how many length bytes we have
    if (data[offset] <= 127) {
        length = (uint32_t) data[offset];

        //we used 1 byte
        offset += 1;
        bytesLeft -= 1;
    } else {
        switch (data[offset]) {
        case 0x81:
            //are there enough bytes left?
            if (bytesLeft < 2) {
                return NULL;
            }

            //fetch length
            length = (uint32_t) data[offset + 1];

            //we used 2 bytes
            offset += 2;
            bytesLeft -= 2;

            break;

        case 0x82:
            //are there enough bytes left?
            if (bytesLeft < 3) {
                return NULL;
            }

            //fetch length
            length = (uint32_t) data[offset + 1];
            length *= 256;
            length += (uint32_t) data[offset + 2];

            //we used 3 bytes
            offset += 3;
            bytesLeft -= 3;

            break;

        case 0x83:
            //are there enough bytes left?
            if (bytesLeft < 4) {
                return NULL;
            }

            //fetch length
            length = (uint32_t) data[offset + 1];
            length *= 256;
            length += (uint32_t) data[offset + 2];
            length *= 256;
            length += (uint32_t) data[offset + 3];

            //we used 4 bytes
            offset += 4;
            bytesLeft -= 4;


            break;

        case 0x84:
            //are there enough bytes left?
            if (bytesLeft < 5) {
                return NULL;
            }

            //fetch length
            length = (uint32_t) data[offset + 1];
            length *= 256;
            length += (uint32_t) data[offset + 2];
            length *= 256;
            length += (uint32_t) data[offset + 3];
            length *= 256;
            length += (uint32_t) data[offset + 4];

            //we used 5 bytes
            offset += 5;
            bytesLeft -= 5;

            break;

        default:
            return NULL; //non-supported length
        }
    }

    //check if there are enough bytes left for the value
    if (length > bytesLeft) {
        return NULL;
    }

    //create a TLV to return
    ptr = (ber_tlv_object_t *) malloc(sizeof (ber_tlv_object_t));
    if (!ptr) {
        return NULL;
    }

    ptr->length = length;
    ptr->tag = tag;
    ptr->value = NULL;

    if (ptr->length > 0) {
        //copy value
        ptr->value = (uint8_t *) malloc(ptr->length + 1);
        if (!ptr->value) {
            free(ptr);
            return NULL;
        }

        memset (ptr->value, 0, ptr->length + 1);
        memcpy (ptr->value, &(data[offset]), ptr->length);
        offset += ptr->length;
    }

    //done
    return ptr;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
bool ber_tlv_is_constructed(ber_tlv_t tlv)
{
    ber_tlv_object_t *ptr = NULL;

    //sanity check
    if (!tlv) {
        return false;
    }

    //grab pointer
    ptr = (ber_tlv_object_t *) tlv;

    //check if tag has BER_TLV_CONSTRUCTED_BIT set
    if ((ptr->tag & BER_TLV_CONSTRUCTED_BIT) != 0) {
        return true;    //constructed
    } else {
        return false;   //primitive
    }
}