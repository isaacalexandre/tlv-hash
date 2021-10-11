/** @file
 *  Application test and validate request by Hash to use the BER-TLV Lib
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "util.h"
#include "llist.h"
#include "libtlv.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define UNUSED(x) (void)(x)
/******************************************************
 *                    Constants
 ******************************************************/
#define BUFFER_INPUT_SIZE 4096
#define BUFFER_HEX_SIZE (BUFFER_INPUT_SIZE/2)
#define INITIAL_OBJ_LEN   13
#define COMPLEX_OBJ_LEN 26

//TVL Initial
const uint8_t tlvObject[INITIAL_OBJ_LEN] = {
    0xE1, 0x0B, 0xC1, 0x03, 0x01, 0x02,
    0x03, 0xC2, 0x00, 0xC3, 0x02, 0xAA,
    0xBB
};
const uint8_t complextlvObject[COMPLEX_OBJ_LEN] = {
    0x6F, 0x18, 0xA5, 0x16, 0x88, 0x01, 0x02, 0x5F, 0x2D, 
    0x02, 0x65, 0x6E, 0x7F, 0x2D, 0x0B, 0xC4, 0x03, 0x04, 
    0x05, 0x03, 0xC5, 0x00, 0xC6, 0x02, 0xAA, 0xBB
};



void usage() {
    printf("usage:\n");
    printf("./apptesthash [options]\n");
    printf("-d      debug enabled [0-1]\n");
    printf("        Default = 0\n\n");
    printf("\r\n");
    exit(1);
}

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
typedef struct {
    int (*func)(uint8_t *,int32_t);
    char *func_name;
} commands_s;
/******************************************************
 *               Static Function Declarations
 ******************************************************/
static int f_tlv_set(uint8_t * p_value, int32_t i32_size);
static int f_tlv_get(uint8_t * p_value, int32_t i32_size);
static int f_tlv_pretty(uint8_t * p_value, int32_t i32_size);

//Function array to use in application
commands_s commands[] = {
    {NULL, "Exit"},
    {f_tlv_set, "[SET] TLV Set - Adiciona uma ou mais tags"},
    {f_tlv_get, "[GET] TLV Get - Requisita uma ou mais tags"},
    {f_tlv_pretty, "[PRY] TLV Pretty - Requisita uma ou mais tags e exibe em forma arvore"},
};

/******************************************************
 *               Variable Definitions
 ******************************************************/
 
/******************************************************
 *               Function Definitions
 ******************************************************/

/******************************************************
 *               Interface functions
 ******************************************************/
/*******************************************************************************
 * Set one or more tags to the system using ISO/IEC 8825 definition
 * @param p_value Array in ASC represeting HEX, structured by ISO/IEC 8825
 ******************************************************************************/
static int f_tlv_set(uint8_t * p_value, int32_t i32_size)
{    
    return ber_tlv_set(p_value, i32_size);
}
/*******************************************************************************
 * Get one or more tags from the system using ISO/IEC 8825 definition
 * @param p_value Array in ASC represeting HEX, structured by ISO/IEC 8825
 ******************************************************************************/
static int f_tlv_get(uint8_t * p_value, int32_t i32_size)
{
    uint32_t u32_tag = 0;
    int32_t i32_aux_size = BUFFER_HEX_SIZE;
    uint32_t ret = 0;
    //Verify if the tag is supported
    if(i32_size > 4)
        return -1;

    //Get the tag
    u32_tag = convert_hex_to_u32(p_value, i32_size);    
    
    ret = ber_tlv_get(p_value, &i32_aux_size, u32_tag);
    if(!ret) {
        //Print information
        printf("\n-> GET: TAG=%02X TLV=",u32_tag);
        for(int16_t i = 0; i < i32_aux_size; i++) 
            printf("%02X",p_value[i]);
    }
    return ret;
}
/*******************************************************************************
 * Print one or more tags from the system using ISO/IEC 8825 definition
 * @param p_value Array in ASC represeting HEX, structured by ISO/IEC 8825
 ******************************************************************************/
static int f_tlv_pretty(uint8_t * p_value, int32_t i32_size)
{
    uint32_t u32_tag = 0;
    int32_t i32_aux_size = BUFFER_HEX_SIZE;
    uint32_t ret = 0;
    //Verify if the tag is supported
    if(i32_size > 4)
        return -1;

    //Get the tag
    u32_tag = convert_hex_to_u32(p_value, i32_size);    
    
    ret = ber_tlv_pretty(p_value, &i32_aux_size, u32_tag);
    if(!ret) {
        //Print information
        printf("\n-> PRETTY: TAG=%02X \n\n%s",u32_tag, p_value);        
    }
    return ret;
}

int main (int argc, char *argv[])
{
    uint32_t u32_cmd_number = (sizeof(commands) / sizeof(commands_s));
    uint32_t i = 0;
    uint32_t u32_option = 0;
    uint32_t u32_ret = 0;
    uint8_t u8_buffer[BUFFER_INPUT_SIZE];
    uint8_t u8_buffer_hex[BUFFER_HEX_SIZE];
    int32_t i32_size = BUFFER_HEX_SIZE;
    int c = 0;
    bool b_debug = 0;
    
    while ((c = getopt (argc, argv, "d:h:stv")) != -1) {
        switch (c) {
        case 'd':
            b_debug = (bool) atoi(optarg);;
            break;
        case 'h':
        case '?':
        default:
            usage();
            break;
        }
    }

    //Initialize BER-TLV Lib
    u32_ret = ber_tlv_init(b_debug);
    if(u32_ret) 
        return 0;

    // Insert the model to system
    u32_ret = f_tlv_set((uint8_t *)tlvObject, INITIAL_OBJ_LEN);
    if(u32_ret) 
        return 0;

    u32_ret = f_tlv_set((uint8_t *)complextlvObject, COMPLEX_OBJ_LEN);
    if(u32_ret) 
        return 0;
        

    while(1) {
        //Print in the console the commands and corresponding number
        for (i = 0; i < u32_cmd_number; i++)
            printf("%4d. %-42s\n", i, commands[i].func_name);
        printf("\n>");
        //Get from the user the selected command
        u32_option = read_ulong();

        //Clear the buffer
        memset(u8_buffer,0x00,BUFFER_INPUT_SIZE);

        //Valide the command
        if (u32_option == 0) {
            printf("See ya\n");
            ber_tlv_terminate();
            return 0;
        } else if (u32_option < u32_cmd_number) {            
            if (commands[u32_option].func != NULL) { //Verify if the function is already populated 
                printf("> Input: ");
                read_string((char*)u8_buffer, BUFFER_INPUT_SIZE); //Get from the user the BER-TLV
                convert_ascii_to_hex(u8_buffer_hex, &i32_size, (char *)u8_buffer, strlen((const char *)u8_buffer)); //Convert to Hex
                u32_ret = commands[u32_option].func(u8_buffer_hex, i32_size); //Use the function in the struct array "commands[]"
                printf("\n\nret = [%d] \n\n", u32_ret);
            }
        } else {
            printf("Invalid function... ");
        }
    }
    return 0;
}
