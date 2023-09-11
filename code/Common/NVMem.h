// Copyright (c) 2002-2010,  Microchip Technology Inc.
//
// Microchip licenses this software to you solely for use with Microchip
// products.  The software is owned by Microchip and its licensors, and
// is protected under applicable copyright laws.  All rights reserved.
//
// SOFTWARE IS PROVIDED "AS IS."  MICROCHIP EXPRESSLY DISCLAIMS ANY
// WARRANTY OF ANY KIND, WHETHER EXPRESS OR IMPLIED, INCLUDING BUT
// NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.  IN NO EVENT SHALL
// MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
// CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, HARM TO YOUR
// EQUIPMENT, COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY
// OR SERVICES, ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED
// TO ANY DEFENSE THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION,
// OR OTHER SIMILAR COSTS.
//
// To the fullest extent allowed by law, Microchip and its licensors
// liability shall not exceed the amount of fees, if any, that you
// have paid directly to Microchip to use this software.
//
// MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE
// OF THESE TERMS.
#ifndef __NVMEM_H__
#define __NVMEM_H__


// PIC32MX1xx and PIC32MX2xx devices
#define FLASH_PAGE_SIZE_PIC32MX1 		1024        // bytes
#define DEV_CONFIG_REG_BASE_ADDRESS 0x9FC00BF0
#define DEV_CONFIG_REG_END_ADDRESS   0x9FC00BFF

#define PAGE_SIZE_PIC32MX1               256        // # of 32-bit Instructions per Page
#define BYTE_PAGE_SIZE_PIC32MX1          (4 * PAGE_SIZE_PIC32MX1) // Page size in Bytes
#define ROW_SIZE_PIC32MX1                32         // # of 32-bit Instructions per Row
#define BYTE_ROW_SIZE_PIC32MX1           (4 * ROW_SIZE_PIC32MX1) // # Row size in Bytes
#define NUM_ROWS_PAGE_PIC32MX1           8              //Number of Rows per Page 




//    // PIC32MX3xx to PIC32MX7xx devices
//    #define FLASH_PAGE_SIZE		 		4096
//    #define DEV_CONFIG_REG_BASE_ADDRESS 0x9FC02FF0
//    #define DEV_CONFIG_REG_END_ADDRESS   0x9FC02FFF

//#define PAGE_SIZE               1024        // # of 32-bit Instructions per Page
//#define BYTE_PAGE_SIZE          (4 * PAGE_SIZE) // Page size in Bytes
//#define ROW_SIZE                128         // # of 32-bit Instructions per Row
//#define BYTE_ROW_SIZE           (4 * ROW_SIZE) // # Row size in Bytes
//#define NUM_ROWS_PAGE           8              //Number of Rows per Page 

#define NVMOP_WORD_PGM          0x4001      // Word program operation
#define NVMOP_PAGE_ERASE        0x4004      // Page erase operation
#define NVMOP_ROW_PGM           0x4003      // Row program operation
#define NVMOP_NOP               0x4000      // NOP operation

#ifdef __cplusplus
extern "C" {
#endif
extern UINT NVMemWriteWord(void* address, UINT data);
extern UINT NVMemErasePage(void* address);
extern UINT NVMemWriteRow(void* address, void* data);
extern UINT NVMemClearError(void);
#ifdef __cplusplus
}
#endif


#endif
