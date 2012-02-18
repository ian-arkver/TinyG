/*
 *  xio_eep.c	- device driver for program memory "files" in eeprom
 * 				- works with avr-gcc stdio library
 *
 * Part of TinyG project
 * Copyright (c) 2011 Alden S. Hart Jr.
 *
 * TinyG is free software: you can redistribute it and/or modify
 * it under the terms of the Creative Commons CC-BY-NC license 
 * (Creative Commons Attribution Non-Commercial Share-Alike license)
 * as published by Creative Commons. You should have received a copy 
 * of the Creative Commons CC-BY-NC license along with TinyG.
 * If not see http://creativecommons.org/licenses/
 *
 * TinyG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 */

#include <stdio.h>						// precursor for xio.h
#include <avr/pgmspace.h>				// precursor for xio.h

#include "xio.h"						// includes for all devices are in here
#include "../xmega/xmega_eeprom.h"

#define EEP ds[XIO_DEV_EEP]				// device struct accessor
#define EEPf fs[XIO_DEV_EEP_OFFSET]		// file extended struct accessor

/* 
 *	xio_init_eep() - initialize and set controls for program memory device 
 */

void xio_init_eep()
{
	// EEPROM file device setup
	xio_init_dev(XIO_DEV_EEP, xio_open_eep, xio_cntl_eep, xio_putc_eep, xio_getc_eep, xio_gets_eep);
	xio_init_file(XIO_DEV_EEP, XIO_DEV_EEP_OFFSET, EEP_INIT_bm);
}

/*	
 *	xio_open_eep() - provide a string address to the program memory device
 *
 *	OK, so this is not really a UNIX open() except for it's moral equiv
 *  Returns a pointer to the stdio FILE struct or -1 on error
 */

struct __file * xio_open_eep(const prog_char *addr)
{
	EEP.flags &= XIO_FLAG_RESET_gm;		// reset flag signaling bits
	EEP.signal = 0;						// reset signal
	EEPf.filebase_P = (PROGMEM char *)addr;	// might want to range check this
//	EEPf.filebase_P = (char *)addr;		// might want to range check this
	EEPf.rd_offset = 0;					// initialize read buffer pointer
	EEPf.wr_offset = 0;					// initialize write buffer pointer
	EEPf.max_offset = EEP_ADDR_MAX;		// initialize write buffer pointer

	EEPROM_DisableMapping();			// EEPROM must be IO mapped
	return(EEP.fdev);					// return pointer to the fdev stream
}

/*	
 *	xio_seek_eep() - position read and write offset in file
 */

int xio_seek_eep(uint32_t offset)
{
	if (offset > EEPf.max_offset) {
		return (XIO_FILE_SIZE_EXCEEDED);
	}
	EEPf.rd_offset = offset;
	EEPf.wr_offset = offset;
	return(XIO_OK);
}

/*	
 *	xio_rewind_eep() - position read and write offset to start of file
 */

int xio_rewind_eep()
{
	EEPf.rd_offset = 0;
	EEPf.wr_offset = 0;
	return(XIO_OK);
}

/*
 *	xio_cntl_eep() - check and set control flags for device
 */

int xio_cntl_eep(const uint32_t control)
{
	(void)xio_cntl(XIO_DEV_EEP, control);
	return (XIO_OK);									// for now it's always OK
}

/* 
 *	xio_putc_eep() - write character to EEPROM
 */

int xio_putc_eep(const char c, struct __file *stream)
{
	uint16_t address = EEP_ADDR_BASE + (uint16_t)EEPf.filebase_P + EEPf.wr_offset;
	EEPROM_WriteByte(address, c);
	++EEPf.wr_offset;
	return (XIO_OK);
}

/* 
 *	xio_puts_eep() - write terminated string to EEPROM
 *
 *	The main difference between this routine and the underlying driver routine
 *	EEPROM_WriteString() is that puts_eep() keeps track of the file location.
 */

int xio_puts_eep(char *buf, struct __file *stream)
{
	uint16_t address = EEP_ADDR_BASE + (uint16_t)EEPf.filebase_P + EEPf.wr_offset;
	EEPf.wr_offset = (uint16_t)EEPROM_WriteString(address, buf, TRUE);
	return (XIO_OK);
}

/*
 *  xio_getc_eep() - read a character from program memory device
 *
 *  Get next character from program memory file.
 *
 *  END OF FILE (EOF)
 *		- the first time you encounter NUL, return ETX
 *		- all subsequent times return NUL
 *	    This allows the higher level stdio routines to return a line that 
 *	    terminates with a NUL, but reads from the end of file will return errors.
 *	  	
 *		- return ETX (as returning NUL is indistinguishable from an error)
 *		- return NUL (this is NOT EOF, which is -1 and signifies and error)
 *
 *  LINEMODE and SEMICOLONS behaviors
 *		- consider <cr> and <lf> to be EOL chars (not just <lf>)
 *		- also consider semicolons (';') to be EOL chars if SEMICOLONS enabled
 *		- convert any EOL char to <lf> to signal end-of-string (e.g. to fgets())
 *
 *  ECHO behaviors
 *		- if ECHO is enabled echo character to stdout
 *		- echo all line termination chars as newlines ('\n')
 *		- Note: putc should expand newlines to <cr><lf>
 */

int xio_getc_eep(struct __file *stream)
{
	if ((EEP.flags & XIO_FLAG_EOF_bm) != 0) {
		EEP.signal = XIO_SIG_EOF;
		return (_FDEV_EOF);
	}
	uint16_t address = EEP_ADDR_BASE + (uint16_t)EEPf.filebase_P + EEPf.rd_offset;
	if ((EEP.c = EEPROM_ReadChar(address)) == NUL) {
		EEP.flags |= XIO_FLAG_EOF_bm;
	}
	++EEPf.rd_offset;
	if (LINEMODE(EEP.flags) == 0) {		// processing is simple if not LINEMODE
		if (ECHO(EEP.flags) != 0) {
			putchar(EEP.c);
		}
		return (EEP.c);
	}
	// now do the LINEMODE stuff
	if (EEP.c == NUL) {					// perform newline substitutions
		EEP.c = '\n';
	} else if (EEP.c == '\r') {
		EEP.c = '\n';
//	} else if ((SEMICOLONS(EEP.flags) != 0) && (EEP.c == ';')) {
//		EEP.c = '\n';
	}
	if (ECHO(EEP.flags) != 0) {
		putchar(EEP.c);
	}
	return (EEP.c);
}

/* 
 *	xio_gets_eep() - get string from EEPROM device
 *
 *	Non-blocking, run-to-completion return a line from memory
 *	Note: LINEMODE flag is ignored. It's ALWAYS LINEMODE here.
 */

int xio_gets_eep(char *buf, const int size)
{
	if ((EEPf.filebase_P) == NULL) {	// return error if no file is open
		return (XIO_FILE_NOT_OPEN);
	}
	EEP.signal = XIO_SIG_OK;			// initialize signal
	if (fgets(buf, size, EEP.fdev) == NULL) {
		EEPf.filebase_P = NULL;
		clearerr(EEP.fdev);
		return (XIO_EOF);
	}
	return (XIO_OK);
}
