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
#define UNUSED(x) (void)(x)
/******************************************************
 *                    Constants
 ******************************************************/
#define BER_TLV_CONSTRUCTED_BIT    0x20
#define BER_TLV_COMPLEX_TAG_BITS   0x1F
#define BER_TLV_CLASS_TAG_BITS     0xC0
/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef void * ber_tlv_t;
typedef void * ber_tlv_constructed_t;
typedef struct {
    uint32_t   tag;
    uint32_t   length;
    uint8_t *  value;
} ber_tlv_object_t;

typedef struct {
    uint32_t   tag;
    uint16_t   num;//Number of children
    uint32_t   p_child_tag[STRUCT_NUM_CHILD];
    uint32_t   length;
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
//Serial func
uint32_t ber_tlv_serialize(uint32_t tag, uint8_t *output, uint32_t *outputLength);
uint32_t ber_tlv_serialize_constructed(uint32_t tag, uint8_t *output, uint32_t *outputLength);
uint32_t ber_tlv_serialize_primitive(uint32_t tag, uint8_t *output, uint32_t *outputLength);
uint32_t ber_tlv_serialize_pretty(uint32_t tag, uint8_t *output, uint32_t *outputLength);
uint32_t ber_tlv_serialize_constructed_pretty(uint32_t tag, uint8_t *output, uint32_t *outputLength, uint32_t *bytes_remains, uint32_t* offset_printf);
uint32_t ber_tlv_serialize_primitive_pretty(uint32_t tag, uint8_t *output, uint32_t *outputLength, uint32_t *bytes_remains, uint32_t* offset_printf);
//Tools
bool ber_tlv_is_constructed(uint32_t tag);
uint32_t ber_tlv_get_num_bytes_tag(uint32_t tag);
uint32_t ber_tlv_get_num_bytes_length(uint32_t len);
uint8_t* ber_tlv_get_type(uint32_t tag);
uint8_t* ber_tlv_get_class(uint32_t tag);
uint32_t ber_tlv_tab_increment_pretty(uint32_t num_tab, uint8_t* str);
//TLV Tools
ber_tlv_t ber_tlv_create_object(void);
ber_tlv_t ber_tlv_parse(uint8_t *data, uint32_t dataLength, uint32_t *bytes_parsed);
void ber_tlv_delete_value(ber_tlv_t tlv);
void ber_tlv_delete_object(ber_tlv_t tlv);
//List Func
uint32_t ber_tlv_push_to_list_primitive(ber_tlv_t tlv);
uint32_t ber_tlv_push_to_list_constructed(ber_tlv_t tlv);
ber_tlv_t ber_tlv_get_from_list_primitive(uint32_t tag);
ber_tlv_constructed_t ber_tlv_get_from_list_constructed(uint32_t tag);
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
    //Clear the lists and free tlvs
    ber_tlv_object_t* ptlv_obj = NULL;

    //Clear list primitives
    struct node *ptr = *tags_primitive;
    while (ptr != NULL) { //Find in the list the tag
        ptlv_obj = (ber_tlv_object_t*)(ptr->data);
        ptr = ptr->next;
        ber_tlv_delete_object(ptlv_obj);
    }
    //Clear Constructed
    ptr= *tags_constructed;
    while (ptr != NULL) { //Find in the list the tag
        free(ptr->data);
        ptr = ptr->next;        
    }
    //Free lists
    llist_free(tags_primitive);
    llist_free(tags_constructed);

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
    uint32_t bytes_parsed;
    ber_tlv_t* ptlv = ber_tlv_parse(p_value, i32_size,&bytes_parsed);   //Allocate in memory the object
    ber_tlv_object_t* ptlv_obj = (ber_tlv_object_t*)ptlv;  
    
    //Sanity
    if(ptlv == NULL || ptlv_obj == NULL)
        goto error;
    //Verify if all byte was parsed
    if(bytes_parsed != (uint32_t)i32_size)
        goto error;

    if(b_debug_enabled) {
        printf("\nTAG:%02X LEN:%d DATA: ",ptlv_obj->tag ,ptlv_obj->length);
        for(uint32_t i=0; i < ptlv_obj->length; i++)
            printf("%02X ",ptlv_obj->value[i]);
    }
    
    //Verify if the TLV is construted 
    if(ber_tlv_is_constructed(ptlv_obj->tag))
        ber_tlv_push_to_list_constructed(ptlv);
    else         
        ber_tlv_push_to_list_primitive(ptlv); //Just store the TLV

    //Delete the object received
    ber_tlv_delete_object(ptlv); 

    return 0;

    error:
    ber_tlv_delete_object(ptlv);
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
uint32_t ber_tlv_get(uint8_t * resp_str, int32_t *i32_resp_size, uint32_t u32_tag)
{
    if(b_debug_enabled) {
        printf("\n\nTAG PRIMITIVES");
        ber_tlv_object_t* ptlv_obj = NULL;
        struct node *curr = *tags_primitive;
        while (curr != NULL) { //Find in the list the tag
            ptlv_obj = (ber_tlv_object_t*)(curr->data);
            curr = curr->next;
            if(b_debug_enabled) {
                printf("\nTAG:%02X LEN:%d DATA: ",ptlv_obj->tag ,ptlv_obj->length);
                for(uint32_t i=0; i < ptlv_obj->length; i++)
                    printf("%02X ",ptlv_obj->value[i]);
            }
        }

        printf("\n\nTAG CONSTRUCTED");
        ber_tlv_constructed_object_t* ptlv_const = NULL;
        struct node *curr2 = *tags_constructed;
        while (curr2 != NULL) { //Find in the list the tag
            if(curr2->data == NULL)
                break;
            ptlv_const = (ber_tlv_constructed_object_t*)(curr2->data);
            curr2 = curr2->next;
            if(b_debug_enabled) {
                printf("\nTAG:%02X LEN:%d ",ptlv_const->tag ,ptlv_const->length);
                for(uint32_t i=0; i < ptlv_const->num; i++)
                    printf("TAG%d:%02X ",i,ptlv_const->p_child_tag[i]);
            }
        }  
         printf("\n\n");
    } 
    return ber_tlv_serialize(u32_tag, resp_str, (uint32_t*)i32_resp_size);
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
uint32_t ber_tlv_pretty(uint8_t * resp_str, int32_t *i32_resp_size, uint32_t u32_tag)
{
    return ber_tlv_serialize_pretty(u32_tag, resp_str, (uint32_t*)i32_resp_size);
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
uint32_t ber_tlv_push_to_list_primitive(ber_tlv_t tlv)
{
    ber_tlv_object_t* ptlv_obj = (ber_tlv_object_t*)tlv;
    ber_tlv_object_t* aux_tlv = NULL;
    //Verify if already exist the tag
    aux_tlv = ber_tlv_get_from_list_primitive(ptlv_obj->tag); 
    if(aux_tlv == NULL) {
        //Push to TLV Primitive List
        llist_push(tags_primitive, (void*)tlv);
    } else {
        //Update the value tag      
        aux_tlv->length = ptlv_obj->length;
        aux_tlv->value = ptlv_obj->value; //Copy pointer
        ptlv_obj->tag = 0;
    }
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
ber_tlv_t ber_tlv_get_from_list_primitive(uint32_t tag)
{
    ber_tlv_object_t* ptlv_obj = NULL;
    struct node *curr = *tags_primitive;
    while (curr != NULL) { //Find in the list the tag
        if(curr->data == NULL)
            break;
        ptlv_obj = (ber_tlv_object_t*)(curr->data);
        curr = curr->next;
        
        if(ptlv_obj->tag == tag)
            return ((ber_tlv_t)ptlv_obj);
    }
    //Not able to find
    return NULL;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
uint32_t ber_tlv_push_to_list_constructed(ber_tlv_t tlv)
{   
    uint32_t u32_len_stored = 0;
    uint32_t u32_bytes_parsed = 0;
    //Aux pointer to storage
    ber_tlv_t p_aux_tlv = NULL;
    ber_tlv_object_t* p_aux_obj_tlv = NULL;
    //Grab pointer
    ber_tlv_object_t* p_obj_tlv = (ber_tlv_object_t*)tlv;

    //Alloc the construct info if was requered
    ber_tlv_constructed_object_t* p_tlc_contructed = ber_tlv_get_from_list_constructed(p_obj_tlv->tag);
    //Check if was null
    if(p_tlc_contructed == NULL) {
        p_tlc_contructed = (ber_tlv_constructed_object_t *) malloc(sizeof(ber_tlv_constructed_object_t));
        //Push to TLV Constructed List
        llist_push(tags_constructed,(void*)p_tlc_contructed);
    }
    
    //Copy information
    p_tlc_contructed->length = p_obj_tlv->length;
    p_tlc_contructed->tag = p_obj_tlv->tag;
    p_tlc_contructed->num = 0;    

    //Store all information inside the TLV
    do{
        //Get node by node in the TLV
        p_aux_tlv = ber_tlv_parse(&p_obj_tlv->value[u32_len_stored], p_obj_tlv->length, &u32_bytes_parsed); //Allocate in memory the object
        p_aux_obj_tlv = (ber_tlv_object_t*)p_aux_tlv;

        if(p_aux_tlv == NULL)
            goto error;

        //Debug        
        if(b_debug_enabled) {
            printf("\nTAG:%02X LEN:%d DATA: ",p_aux_obj_tlv->tag ,p_aux_obj_tlv->length);
            for(uint32_t i=0; i < p_aux_obj_tlv->length; i++)
                printf("%02X ",p_aux_obj_tlv->value[i]);
        }

        //Store the tag into constructed struct and increment num_of_tags
        p_tlc_contructed->p_child_tag[p_tlc_contructed->num++] = p_aux_obj_tlv->tag;

        //Verify if is constructed TLV
        if(ber_tlv_is_constructed(p_aux_obj_tlv->tag)) {
            ber_tlv_push_to_list_constructed(p_aux_tlv); //Recursive function
        } else {
            //Push to the list if is primitive
            ber_tlv_push_to_list_primitive(p_aux_tlv);
        }
        
        //Increment the offset
        u32_len_stored += u32_bytes_parsed;

    }while(u32_len_stored < p_tlc_contructed->length);

    return 0;

    error:
    p_tlc_contructed->num = 0;
    ber_tlv_delete_object(p_aux_tlv);
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
ber_tlv_constructed_t ber_tlv_get_from_list_constructed(uint32_t tag)
{
    ber_tlv_constructed_object_t* ptlv_obj = NULL;
    struct node *curr = *tags_constructed;
    while (curr != NULL) { //Find in the list the tag
        if(curr->data == NULL)
            break;
        ptlv_obj = (ber_tlv_constructed_object_t*)(curr->data);
        curr = curr->next;
        
        if(ptlv_obj->tag == tag)
            return ((ber_tlv_constructed_t)ptlv_obj);
    }
    //Not able to find
    return NULL;
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

    //Initialize
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
void ber_tlv_delete_value(ber_tlv_t tlv)
{
    ber_tlv_object_t *ptr = NULL;

    //Sanity check
    if (!tlv) 
        return;
    
    //Grab pointer
    ptr = (ber_tlv_object_t *) tlv;

    //Free memory
    if (ptr->value) {
        free(ptr->value);
        ptr->length = 0;
    }

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
    //Sanity check
    if (!tlv) 
        return;    

    //Delete value
    ber_tlv_delete_value(tlv);

    //Free memory
    if(tlv != NULL)
        free(tlv);
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
uint32_t ber_tlv_serialize(uint32_t tag, uint8_t *output, uint32_t *outputLength)
{
    uint32_t ret = 0;
    //Verify if is constructed
    if (ber_tlv_is_constructed(tag))
        ret = ber_tlv_serialize_constructed(tag, output, outputLength);
    else 
        ret = ber_tlv_serialize_primitive(tag, output, outputLength);
    
    return ret;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
uint32_t ber_tlv_serialize_primitive(uint32_t tag, uint8_t *output, uint32_t *outputLength)
{
    uint32_t offset = 0;
    uint32_t bytes_offset = 0;
    uint32_t length_offset = 0;

    //Get from the list the primitive
    ber_tlv_object_t* p_tlv = ber_tlv_get_from_list_primitive(tag);
    if (p_tlv == NULL)
        goto error;

    // Verify if fits the tag
    if (p_tlv->length + 8 >  *outputLength)
        goto error;

    //Parse the buffer    
    bytes_offset = ber_tlv_get_num_bytes_tag(p_tlv->tag);
    if (bytes_offset == 0)
        goto error;
    else
        bytes_offset--; //To fit in the algoritm bellow
    //Parse tag
    for (int16_t i = bytes_offset; i >= 0; i--)
        output[offset++] =  (uint8_t)((uint32_t)(p_tlv->tag >> (i*8))& 0xFF); //Put in the buffer the appropriate tag

    //Parse length
    length_offset = ber_tlv_get_num_bytes_length(p_tlv->length);
    //Verify the length
    if(length_offset == 0)
        goto error;
    //Need put one extra by before the length
    if (length_offset > 1)
        output[offset++] = (uint8_t)(0x80 | (length_offset-1));
    //Normalize the length
    length_offset--;
    //Populate the array
    for (int16_t i = length_offset; i >= 0; i--)
        output[offset++] =  (uint8_t)((uint32_t)(p_tlv->length >> (i*8))& 0xFF); 

    //Verify the tag length and copy data
    if (p_tlv->length > 0) {
        //Alocate and copy data
        memset (&output[offset], 0, p_tlv->length);
        memcpy (&output[offset], p_tlv->value, p_tlv->length);
        offset += p_tlv->length;
    }

    //Update the length parsed
    if (outputLength) {
        *outputLength = offset;
    }
    return 0;

    error:
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
uint32_t ber_tlv_serialize_constructed(uint32_t tag, uint8_t *output, uint32_t *outputLength)
{
    uint32_t offset = 0;
    uint32_t bytes_offset = 0;
    uint32_t length_offset = 0;
    uint32_t ret = 0;
    uint32_t aux_output_length = 0;
    //Sanity
    if(outputLength == NULL)
        goto error;

    //Get the struct constructed
    ber_tlv_constructed_object_t* p_constrc =  ber_tlv_get_from_list_constructed(tag);
    if(p_constrc == NULL) 
        goto error;

    //Verify if fits
    if(p_constrc->length + 8 > *outputLength)
        goto error;

    //Parse the buffer    
    bytes_offset = ber_tlv_get_num_bytes_tag(p_constrc->tag);
    if (bytes_offset == 0)
        goto error;
    else
        bytes_offset--; //To fit in the algoritm bellow

    //Parse tag
    for (int16_t i = bytes_offset; i >= 0; i--)
        output[offset++] =  (uint8_t)((uint32_t)(p_constrc->tag >> (i*8))& 0xFF); //Put in the buffer the appropriate tag

    //Parse length
    length_offset = ber_tlv_get_num_bytes_length(p_constrc->length);
    //Verify the length
    if(length_offset == 0)
        goto error;
    //Need put one extra by before the length
    if (length_offset > 1)
        output[offset++] = (uint8_t)(0x80 | (length_offset-1));        
    //Normalize the length
    length_offset--;
    
    //Populate the array
    for (int16_t i = length_offset; i >= 0; i--)
        output[offset++] =  (uint8_t)((uint32_t)(p_constrc->length >> (i*8))& 0xFF); 

    //Populate with primitive and constructs
    for(uint16_t i =0; i < p_constrc->num; i++) {        
        //Calculate how many bytes remains
        aux_output_length = *outputLength - offset;
        //Verify if is constructed
        if(ber_tlv_is_constructed(p_constrc->p_child_tag[i]))
            ret = ber_tlv_serialize_constructed(p_constrc->p_child_tag[i], &output[offset], &aux_output_length);//Recursive function
        else
            ret = ber_tlv_serialize_primitive(p_constrc->p_child_tag[i], &output[offset], &aux_output_length);
        //Verify if parse rigth
        if(ret)
            goto error;
        //Sum number of the bytes parsed
        offset += aux_output_length;
    }
    //Update the length
    *outputLength = offset;
    return ret;

    error:
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
uint32_t ber_tlv_serialize_pretty(uint32_t tag, uint8_t *output, uint32_t *outputLength)
{
    uint32_t ret = 0;
    uint32_t offset_print = 0;
    uint32_t byte_remains = *outputLength;
    *outputLength = 0;

    //Clear the buffer
    memset(output, 0x00, *outputLength);
    //Verify if is constructed
    if (ber_tlv_is_constructed(tag))
        ret = ber_tlv_serialize_constructed_pretty(tag, &output[0], outputLength, &byte_remains, &offset_print);
    else 
        ret = ber_tlv_serialize_primitive_pretty(tag, output, outputLength, &byte_remains, &offset_print);
    
    return ret;
}
/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
uint32_t ber_tlv_serialize_constructed_pretty(uint32_t tag, uint8_t *output, uint32_t *outputLength, uint32_t *bytes_remains, uint32_t* offset_printf)
{
    uint32_t offset = 0;
    uint32_t ret = 0;
    uint32_t aux_output_length = 0;

    //Sanity
    if(outputLength == NULL || bytes_remains == NULL) 
        goto error;

    //Get the struct constructed
    ber_tlv_constructed_object_t* p_constrc =  ber_tlv_get_from_list_constructed(tag);
    if(p_constrc == NULL) 
        goto error;

    // Verify if fits the tag
    if (p_constrc->length + 50 >  *bytes_remains)
        goto error;
   
    //Populate the output with pretty string
    offset += ber_tlv_tab_increment_pretty(*offset_printf, &output[offset]);
    sprintf((void *)&output[offset],"TAG – 0x%02X (%s,%s)\r\n", p_constrc->tag, ber_tlv_get_class(p_constrc->tag), ber_tlv_get_type(p_constrc->tag));
    offset += strlen((const char *)&output[offset]);
    
    offset += ber_tlv_tab_increment_pretty(*offset_printf, &output[offset]);
    sprintf((void *)&output[offset],"LEN – %d \r\n", p_constrc->length);
    offset += strlen((const char *)&output[offset]);
    
    //Empty line
    sprintf((void *)&output[offset],"\r\n");
    offset += strlen((const char *)&output[offset]);

    //Increment the offset
    *offset_printf += 1;
    //Populate with primitive and constructs
    for(uint16_t i =0; i < p_constrc->num; i++) {
        //Verify if is constructed
        if(ber_tlv_is_constructed(p_constrc->p_child_tag[i]))
            ret = ber_tlv_serialize_constructed_pretty(p_constrc->p_child_tag[i], &output[offset], &aux_output_length, bytes_remains, offset_printf); //Recursive function
        else
            ret = ber_tlv_serialize_primitive_pretty(p_constrc->p_child_tag[i], &output[offset], &aux_output_length, bytes_remains, offset_printf);

        //Refresh the strng len
        offset += aux_output_length;
        //Verify if parse rigth
        if(ret)
            goto error;
    }
    
    //Decrement the offset
    *offset_printf -= 1;

    //Update the length
    *outputLength = offset;
    return ret;

    error:
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
uint32_t ber_tlv_serialize_primitive_pretty(uint32_t tag, uint8_t *output, uint32_t *outputLength, uint32_t *bytes_remains, uint32_t* offset_printf)
{
    uint32_t offset = 0;

    //Get from the list the primitive
    ber_tlv_object_t* p_tlv = ber_tlv_get_from_list_primitive(tag);
    if (p_tlv == NULL)
        goto error;

    // Verify if fits the tag
    if (p_tlv->length + 8 >  *bytes_remains)
        goto error;

    //Populate the output with pretty string
    offset += ber_tlv_tab_increment_pretty(*offset_printf, &output[offset]);
    sprintf((void *)&output[offset],"TAG – 0x%02X (%s,%s)\r\n", p_tlv->tag, ber_tlv_get_class(p_tlv->tag), ber_tlv_get_type(p_tlv->tag));
    offset += strlen((const char *)&output[offset]);
    
    offset += ber_tlv_tab_increment_pretty(*offset_printf, &output[offset]);
    sprintf((void *)&output[offset],"LEN – %d \r\n", p_tlv->length);
    offset += strlen((const char *)&output[offset]);
    
    offset += ber_tlv_tab_increment_pretty(*offset_printf, &output[offset]);
    sprintf((void *)&output[offset],"VAL – ");
    offset += strlen((const char *)&output[offset]);
    
    //Verify the tag length and copy data
    if (p_tlv->length > 0) {
        for (uint16_t i = 0; i < p_tlv->length; i++) {
            sprintf((void *)&output[offset],"0x%02X ",p_tlv->value[i]);
            offset+=5;
        }
    }
    //Empty line
    sprintf((void *)&output[offset],"\r\n\r\n");
    offset += strlen((const char *)&output[offset]);
   
    //Update the length parsed
    if (outputLength) {
        *outputLength = offset;
    }
    if (bytes_remains) {
        *bytes_remains -= offset;
    }
    
    return 0;

    error:
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
ber_tlv_t ber_tlv_parse(uint8_t *data, uint32_t dataLength, uint32_t *bytes_parsed)
{
    ber_tlv_object_t *ptr = NULL;
    uint32_t offset = 0;
    uint16_t u16_count_len = 0;
    UNUSED(dataLength);

    //Sanity checks
    if (!data) 
        goto error;

    //Allocate memory
    ptr = (ber_tlv_object_t *) malloc(sizeof (ber_tlv_object_t));
    if (!ptr) 
       goto error;    

    //Set 1 byte tag
    ptr->tag = data[offset];

    //Verify if tag have more then 1 Byte and increment offset
    if ((data[offset++] & 0x1F) == 0x1F) {
        do{ //Read the othr bytes tag
            ptr->tag <<= 8;//Shift
            ptr->tag |= data[offset++];
        }while(offset < 4 && (data[offset-1] & 0x80) != 0);//Tag max length and verify if have another byte
    } 
    //Check the length
    if (data[offset] <= 127) { //Only one byte of length
        ptr->length = (uint32_t)data[offset++]; //Increment to the data
    } else if ((data[offset] >= 0x81) && (data[offset] <= 0x84)) {//Verify how many bytes the TLV have
        //Increment the length until finish the bytes   
        u16_count_len = (data[offset] & 0x0F);//Get the last nibble of the first byte to analyse how many bytes remains
        do{
            ptr->length <<= 8;//Shit
            ptr->length |= data[++offset];//Increment and copy to the length
            u16_count_len--;
        }while(u16_count_len > 0);
        offset++; //Increment to the data */
    } else {
        goto error; //Length not supported
    }

    //Verify the tag length
    if (ptr->length > 0) {
        //Alocate and copy data
        ptr->value = (uint8_t *) malloc(ptr->length + 1);
        if (ptr->value == NULL) 
            goto error;        

        memset (ptr->value, 0, ptr->length + 1);
        memcpy (ptr->value, &(data[offset]), ptr->length);
        offset += ptr->length;
    }
    
    //Update how many bytes actually parsed
    if (bytes_parsed) {
        *bytes_parsed = offset;
    }

    return ptr;
    
    error: 
    if(ptr->value != NULL)   
        free(ptr->value);
    if(ptr != NULL)     
        free(ptr);
    return NULL;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
bool ber_tlv_is_constructed(uint32_t tag)
{
    //Check if tag has BER_TLV_CONSTRUCTED_BIT set
    for(int8_t i = 24; i >= 8; i-=8) { //Verify if have more then 1 byte tag
        if((tag & (BER_TLV_COMPLEX_TAG_BITS << i)) != 0) 
            return ((tag & (BER_TLV_CONSTRUCTED_BIT << i)) != 0 ? true : false);
    }

    //If have just 1 byte
    return ((tag & BER_TLV_CONSTRUCTED_BIT) != 0 ? true : false);   
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
uint32_t ber_tlv_get_num_bytes_length(uint32_t len)
{
     uint32_t ret = 0;
    //Verify how many bytes have length
    switch(len) {
        case 0x00 ... 0x7F:
            ret = 1;
            break;
        case 0x80 ... 0x7FFF :
            ret = 2;
            break;
        case 0x8000 ... 0x7FFFFF :
            ret = 3;
            break;
        case 0x800000 ... 0xFFFFFFFF:
            ret = 4;
            break;
        default:
            ret = 0;
            break;
    }   
    return ret;
}

/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
uint32_t ber_tlv_get_num_bytes_tag(uint32_t tag)
{
     uint32_t ret = 1;//By default
    //Verify how many bytes have tag
    for(int8_t i = 24; i >= 8; i-=8) { //Verify if have more then 1 byte tag
        if((tag & (BER_TLV_COMPLEX_TAG_BITS << i)) != 0) 
            return ((i+8)/8);//Return what byte was find the complex tag bits
    }
    return ret;
}
/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
uint8_t* ber_tlv_get_class(uint32_t tag)
{
    uint32_t u32_aux = tag;
    //Check if tag has BER_TLV_CONSTRUCTED_BIT set
    for(int8_t i = 24; i >= 8; i-=8) { //Verify if have more then 1 byte tag
        if((tag & (BER_TLV_COMPLEX_TAG_BITS << i)) != 0) {
            u32_aux = (tag & (BER_TLV_CLASS_TAG_BITS << i));
            u32_aux >>= i;//Shit to the first byte
        }            
    }

    //If have just 1 byte
    u32_aux = u32_aux & BER_TLV_CLASS_TAG_BITS;
    u32_aux >>= 6;

    switch(u32_aux) {
        case 0:
            return (uint8_t*)"Universal Class\0";
        case 1:
            return (uint8_t*)"Application Class\0";
        case 2:
            return (uint8_t*)"Constext-specific Class\0";
        case 3:
            return (uint8_t*)"Private Class\0";
        default:
            return (uint8_t*)"Application Class\0";    
    }    
}
/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
uint8_t* ber_tlv_get_type(uint32_t tag)
{
    return (ber_tlv_is_constructed(tag) == true ? (uint8_t*)"Constructed\0" : (uint8_t*)"Primitive");
}
/****************************************************************************************
 * \brief
 *
 * \param
 *
 * \return
 *
 ***************************************************************************************/
uint32_t ber_tlv_tab_increment_pretty(uint32_t num_tab, uint8_t* str)
{
    uint32_t offset = 0;
    for (uint32_t i = 0; i < num_tab; i++) {
        sprintf((void*)&str[offset], "      ");
        offset += 6;
    }
    return offset;
}