// Bootloader definitions
// (C) 2023-09-09 by Daniel Porzig

#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#include <GenericTypeDefs.h>

// PIC32MX1xx and PIC32MX2xx devices
#define FLASH_PAGE_SIZE_PIC32MX1		 1024
#define DEV_CONFIG_REG_BASE_ADDRESS 0x9FC00BF0
#define DEV_CONFIG_REG_END_ADDRESS   0x9FC00BFF

#define APP_FWSTRING_BASE_ADDRESS       0x9D00F000 
#define BOOTCODE_FWSTRING_BASE_ADDRESS 	0x9D003000 

#define PROGRAM_FLASH_END_ADRESS (0x9D000000+BMXPFMSZ-1)

//-------User configurable macros begin---------
#define MAJOR_VERSION 1
#define MINOR_VERSION 0


/* APP_FLASH_BASE_ADDRESS and APP_FLASH_END_ADDRESS reserves program Flash for the application*/ 
/* Rule: 
 		1)The memory regions kseg0_program_mem, kseg0_boot_mem, exception_mem and 
 		kseg1_boot_mem of the application linker script must fall with in APP_FLASH_BASE_ADDRESS 
 		and APP_FLASH_END_ADDRESS
 		 
 		2)The base address and end address must align on  4K address boundary */
 		
#define APP_FLASH_BASE_ADDRESS 	0x9D008000 
#define APP_FLASH_END_ADDRESS   PROGRAM_FLASH_END_ADRESS


// define destination address of first word from image
#define APP_FLASH_START_ADDRESS 	0x9D008180

/* Address of  the Flash from where the application starts executing */
/* Rule: Set APP_FLASH_BASE_ADDRESS to _RESET_ADDR value of application linker script*/

// For PIC32MX1xx and PIC32MX2xx Controllers only
#define USER_APP_RESET_ADDRESS 	(0x9D008000 + 0x1000)

#define BOOTCODE_CONFIG_EEPROM_ADD      (511 * 128)     // last EEPROM page

#define MAGIC_NUMBER_VALID_APP      0xF25224D8 
#define MAGIC_NUMBER_UPDATE_REQ     0x9E6FED87  
#define MAGIC_NUMBER_INVALID_APP    0x00000000 


typedef struct cfg_bootcode_TD
{
    DWORD magicnumber;

}cfg_bootcode_struct;




#endif