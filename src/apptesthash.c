/** @file
 *  Application test and validate request by Hash to use the BER-TLV Lib
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#define BUFFER_INPUT_SIZE 1024
#define BUFFER_HEX_SIZE (BUFFER_INPUT_SIZE/2)
#define INITIAL_OBJ_LEN   13

//TVL Initial
const uint8_t tlvObject[INITIAL_OBJ_LEN] = {
  0xE1, 0x0B, 0xC1, 0x03, 0x01, 0x02,
  0x03, 0xC2, 0x00, 0xC3, 0x02, 0xAA,
  0xBB
};

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
    UNUSED(p_value);
    UNUSED(i32_size);
    return 0;
}
/*******************************************************************************
 * Print one or more tags from the system using ISO/IEC 8825 definition
 * @param p_value Array in ASC represeting HEX, structured by ISO/IEC 8825
 ******************************************************************************/
static int f_tlv_pretty(uint8_t * p_value, int32_t i32_size)
{
    for(int32_t i=0;i<i32_size;i++)
        printf("%02X ",p_value[i]);
    return 0;
}

int main (int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    uint32_t u32_cmd_number = (sizeof(commands) / sizeof(commands_s));
    uint32_t i = 0;
    uint32_t u32_option = 0;
    uint32_t u32_ret = 0;
    uint8_t u8_buffer[BUFFER_INPUT_SIZE];
    uint8_t u8_buffer_hex[BUFFER_HEX_SIZE];
    int32_t i32_size = BUFFER_HEX_SIZE;

    //Initialize BER-TLV Lib
    u32_ret = ber_tlv_init(true);
    if(u32_ret) 
        return 0;

    // Insert the model to system
    u32_ret = f_tlv_set((uint8_t *)tlvObject, INITIAL_OBJ_LEN);
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
                printf("ret = [%d] \n\n", u32_ret);
            }
        } else {
            printf("Invalid function... ");
        }
    }
    return 0;
}
