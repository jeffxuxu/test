/*--------------------------------------------------------------------------
CTYPE.H

Prototypes for character functions.
Copyright (c) 1988-2002 Keil Elektronik GmbH and Keil Software, Inc.
All rights reserved.
--------------------------------------------------------------------------*/

#ifndef __CTYPE_H__
#define __CTYPE_H__


extern unsigned char isalpha (unsigned char);
extern unsigned char isalnum (unsigned char);
extern unsigned char iscntrl (unsigned char);
extern unsigned char isdigit (unsigned char);
extern unsigned char isgraph (unsigned char);
extern unsigned char isprint (unsigned char);
extern unsigned char ispunct (unsigned char);
extern unsigned char islower (unsigned char);
extern unsigned char isupper (unsigned char);
extern unsigned char isspace (unsigned char);
extern unsigned char isxdigit (unsigned char);
//extern unsigned char tolower (unsigned char);
extern unsigned char toupper (unsigned char);
extern unsigned char toint (unsigned char);

#define _tolower(c) ( (c)-'A'+'a' )
#define _toupper(c) ( (c)-'a'+'A' )
#define toascii(c)  ( (c) & 0x7F )

#define tolower _tolower

#endif
