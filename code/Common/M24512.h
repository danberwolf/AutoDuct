// I²C EEPROM driver
// (C) 2023-09-09 by Daniel Porzig

#ifndef __M24512_H_
#define __M24512_H_

#include <stdlib.h>
#include <stdio.h>
#include "HardwareProfile.h"
#include "GenericTypeDefs.h"
#include "Compiler.h"
#include <plib.h>
#include <proc/p32mx150f128b.h>

#define EEPROM_PAGE_SIZE		128		// Pagesize in bytes


BYTE EEPROM_check_busy();
void EEPROM_init(BYTE addr);
BYTE EEPROM_read(WORD address, BYTE *data, WORD length);
BYTE EEPROM_write(WORD address, BYTE *data, WORD length);
BYTE EEPROM_getPageSize();



#endif

