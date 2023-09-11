// I²C EEPROM driver
// (C) 2023-09-09 by Daniel Porzig

#include "M24512.h"

// Driver File for M24512 serial I²C EEPROM
// 512 kBit total Memory Size 
// 128 Bytes Page Size

static BYTE _eeaddr;

// set i²c address of EEPROM IC
void EEPROM_init(BYTE addr)
{
    _eeaddr = addr;
    
    // default address with all address pins @GND: 0x50
}



//check if EEPROM is still busy with a writing operation
BYTE EEPROM_check_busy()
{
    StartI2C1();        // start condition
    IdleI2C1();     //Wait to complete    

    MasterWriteI2C1( (_eeaddr << 1) | 0x00  );      // send slave address (write)    
    
    IdleI2C1();		//Wait to complete

    if( I2C1STATbits.ACKSTAT == 0 )	//eeprom has acknowledged
    {
        StopI2C1();	//Send the Stop condition
        IdleI2C1();	//Wait to complete
        return 0;
    }
    else
    {
        StopI2C1();	//Send the Stop condition
        IdleI2C1();	//Wait to complete
        return 1;       
        
    }
    
}

// perform a read opertion
BYTE EEPROM_read(WORD address, BYTE *data, WORD length)
{
	BYTE addhl[2];
    WORD i;

    while(EEPROM_check_busy());
    
	addhl[0] = (address >> 8) & 0xff;	
	addhl[1] = address  & 0x00ff;

    StartI2C1();        // start condition
    IdleI2C1();     //Wait to complete    

    MasterWriteI2C1( (_eeaddr << 1) | 0x00  );      // send slave address (write)    
    IdleI2C1();		//Wait to complete

    MasterWriteI2C1( addhl[0] );    
    IdleI2C1();		//Wait to complete
    MasterWriteI2C1( addhl[1] );    
    IdleI2C1();		//Wait to complete
    
    StartI2C1();        // start condition
    IdleI2C1();     //Wait to complete    
    
    MasterWriteI2C1( (_eeaddr << 1) | 0x01  );      // send slave address (read)    
    IdleI2C1();		//Wait to complete
    
    for(i=0; i<length; i++)
    {
        data[i] = MasterReadI2C1();    
        IdleI2C1();     //Wait to complete    
        
        if(i == (length - 1))
        {
            NotAckI2C1();  
            IdleI2C1();     //Wait to complete  
        }
        else
        {
            AckI2C1(); 
            IdleI2C1();     //Wait to complete  
        }
    }
    
    StopI2C1();	//Send the Stop condition
    IdleI2C1();	//Wait to complete
    return 1;       
    
}



// perform a write operation (also deals with page boundary crossing)
BYTE EEPROM_write(WORD address, BYTE *data, WORD length)
{

	WORD add;
	BYTE b2w,i;
	BYTE *bptr = data;
	BYTE offs;
	BYTE addhl[2];
	add = address;

    // calculate byte offset from page boundary
    offs = address % EEPROM_PAGE_SIZE;

	while(length)
	{
        while(EEPROM_check_busy());
        
        
		addhl[0] = (add >> 8) & 0xff;	
		addhl[1] = add  & 0x00ff;

		// get number of bytes to write
		if(length > EEPROM_PAGE_SIZE)
			b2w = EEPROM_PAGE_SIZE;
		else
			b2w = length;

		// make sure we don't cross page boundary
        if(b2w > (EEPROM_PAGE_SIZE - offs))
        {
            b2w = (EEPROM_PAGE_SIZE - offs);
        }

		// write bytes to page
        
        StartI2C1();        // start condition
        IdleI2C1();     //Wait to complete    

        MasterWriteI2C1( (_eeaddr << 1) | 0x00  );      // send slave address (write)    
        IdleI2C1();		//Wait to complete

        MasterWriteI2C1( addhl[0] );    
        IdleI2C1();		//Wait to complete
        MasterWriteI2C1( addhl[1] );    
        IdleI2C1();		//Wait to complete

        for(i=0; i<b2w; i++)
        {
            MasterWriteI2C1( *bptr );    // send data byte
            IdleI2C1();		//Wait to complete     
            bptr++;
        }
        
        StopI2C1();	//Send the Stop condition
        IdleI2C1();	//Wait to complete        

		// increase page address
		add += b2w;

		// decrease remaining bytes
		length -= b2w;

        // next writes will always be page aligned
        offs = 0;

	}
	return 1;
}

// return page size
BYTE EEPROM_getPageSize()
{
    return EEPROM_PAGE_SIZE;
}


