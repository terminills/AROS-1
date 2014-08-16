/***************************************************************************

 NListviews.mcp - New Listview MUI Custom Class Preferences
 Registered MUI class, Serial Number: 1d51 (0x9d510001 to 0x9d51001F
                                            and 0x9d510101 to 0x9d51013F)

 Copyright (C) 1996-2001 by Gilles Masson
 Copyright (C) 2001-2014 NList Open Source Team

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 NList classes Support Site:  http://www.sf.net/projects/nlist-classes

 $Id$

***************************************************************************/

#if !defined(__MORPHOS__)
const unsigned long icon32[] = {
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x0d000000, 0x7320396e, 0xec304e89, 0xff3f5d99, 0xff4664a0, 0xff3f5d99, 0xec304e89, 0x7320396e, 0x0d000000, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x1b000000, 0xac2b4479, 0xff43619d, 0xff5775b1, 0xff617fbb, 0xff5775b1, 0xff43619d, 0xac2b4479, 0x1b000000, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x2a000000, 0xcd314b80, 0xff4e6ca8, 0xff6a88c4, 0xff7d9bd7, 0xff6a88c4, 0xff4e6ca8, 0xcd314b80, 0x2a000000, 0x00ff00ff, 0x04000000, 0x64000000, 0x07000000, 0x03000000, 0x48000000, 0x03000000, 0x00ff00ff, 0x03000000, 0x4c000000, 0x03000000, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x2a000000, 0xe538558d, 0xff516fab, 0xff6f8dc9, 0xff89a7e3, 0xff6f8dc9, 0xff516fab, 0xe538558d, 0x2a000000, 0x00ff00ff, 0x0b000000, 0x84000000, 0x0e000000, 0x09000000, 0x67000000, 0x3d000000, 0x72000000, 0x4c000000, 0xa5000000, 0x5b000000, 0x03000000, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x2a000000, 0xfb3e5b98, 0xff4a68a4, 0xff6785c0, 0xff7f9ddb, 0xff6785c0, 0xff4a68a4, 0xfb3e5b98, 0x2a000000, 0x00ff00ff, 0x12000000, 0x88000000, 0x15000000, 0x0f000000, 0x80000000, 0x40000000, 0x87000000, 0x58000000, 0x87000000, 0x17000000, 0x03000000, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x2a000000, 0xff22417e, 0xff38548c, 0xff687a9e, 0xff97a9ce, 0xff687a9e, 0xff38548c, 0xff22417e, 0x2a000000, 0x00ff00ff, 0x14000000, 0x8b000000, 0x80000000, 0x6d000000, 0x86000000, 0x52000000, 0x71000000, 0x8a000000, 0x74000000, 0x6a000000, 0x08000000, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x2a000000, 0xff102f6b, 0xff23417c, 0xff334771, 0xff41506d, 0xff334771, 0xff23417c, 0xff102f6b, 0x2a000000, 0x00ff00ff, 0x0d000000, 0x13000000, 0x18000000, 0x17000000, 0x16000000, 0x1d000000, 0x22000000, 0x20000000, 0x19000000, 0x0f000000, 0x04000000, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x1b000000, 0x38000000, 0x54000000, 0x54000000, 0x54000000, 0x54000000, 0x54000000, 0x38000000, 0x1b000000, 0x00ff00ff, 0x06000000, 0x0c000000, 0x11000000, 0x11000000, 0x0d000000, 0x0e000000, 0x0d000000, 0x10000000, 0x0f000000, 0x09000000, 0x04000000, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x0d000000, 0x1b000000, 0x2a000000, 0x2a000000, 0xb9000000, 0x2a000000, 0x2a000000, 0x1b000000, 0x0d000000, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x0d000000, 0x7320396e, 0xec304e89, 0xff3f5d99, 0xff4664a0, 0xff3f5d99, 0xec304e89, 0x7320396e, 0x0d000000, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0xab000000, 0x00ff00ff, 0x00ff00ff, 0x1b000000, 0xac2b4479, 0xff43619d, 0xff5775b1, 0xff617fbb, 0xff5775b1, 0xff43619d, 0xac2b4479, 0x1b000000, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x2a000000, 0xcd314b80, 0xff4e6ca8, 0xff6a88c4, 0xff7d9bd7, 0xff6a88c4, 0xff4e6ca8, 0xcd314b80, 0x2a000000, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0xab000000, 0x00ff00ff, 0xab000000, 0x2a000000, 0xe538558d, 0xff516fab, 0xff6f8dc9, 0xff89a7e3, 0xff6f8dc9, 0xff516fab, 0xe538558d, 0x2a000000, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x2a000000, 0xfb3e5b98, 0xff4a68a4, 0xff6785c0, 0xff7f9ddb, 0xff6785c0, 0xff4a68a4, 0xfb3e5b98, 0x2a000000, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x2a000000, 0xff22417e, 0xff38548c, 0xff687a9e, 0xff97a9ce, 0xff687a9e, 0xff38548c, 0xff22417e, 0x2a000000, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x2a000000, 0xff102f6b, 0xff23417c, 0xff334771, 0xff41506d, 0xff334771, 0xff23417c, 0xff102f6b, 0x2a000000, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x1b000000, 0x38000000, 0x54000000, 0x54000000, 0x54000000, 0x54000000, 0x54000000, 0x38000000, 0x1b000000, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x0d000000, 0x1b000000, 0x2a000000, 0x2a000000, 0x2a000000, 0x2a000000, 0x2a000000, 0x1b000000, 0x0d000000, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
};
#else // !__MORPHOS__
const unsigned char icon32[] =
{
#if defined(__PPC__)
	0x00, 0x00, 0x00, 0x18,
	0x00, 0x00, 0x00, 0x14,
	'B', 'Z', '2', '\0',
	0x00, 0x00, 0x01, 0xf1,
#else
	0x18, 0x00, 0x00, 0x00,
	0x14, 0x00, 0x00, 0x00,
	'B', 'Z', '2', '\0',
	0xf1, 0x01, 0x00, 0x00,
#endif

	0x42, 0x5A, 0x68, 0x39, 0x31, 0x41, 0x59, 0x26, 0x53, 0x59, 0x6F, 0x0B, 0xC4, 0x06, 0x00, 0x00,
	0xD6, 0x7F, 0xFF, 0xCD, 0xEF, 0xFF, 0xEA, 0x58, 0x18, 0xE8, 0x63, 0xED, 0xDD, 0x76, 0xCA, 0x24,
	0xDF, 0xBE, 0x37, 0xC7, 0xFE, 0x00, 0xEB, 0x46, 0xEC, 0x20, 0x28, 0x44, 0x23, 0x00, 0x88, 0x0A,
	0x04, 0x00, 0x08, 0xC0, 0x01, 0xD0, 0x38, 0x01, 0xC0, 0x00, 0x68, 0x00, 0x06, 0x40, 0x00, 0x00,
	0x19, 0x0D, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x52, 0x29, 0xEA, 0x04, 0xDA,
	0x1A, 0x68, 0x13, 0x04, 0xDA, 0x69, 0x34, 0xC0, 0x01, 0x0C, 0x68, 0x09, 0xA0, 0x61, 0x18, 0x00,
	0x00, 0x9A, 0x60, 0x10, 0x68, 0xC1, 0xC0, 0x00, 0x68, 0x00, 0x06, 0x40, 0x00, 0x00, 0x19, 0x0D,
	0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x50, 0x93, 0x4A, 0x68, 0x7A, 0x9E, 0xA0,
	0x64, 0xD3, 0x20, 0xD1, 0xA0, 0x64, 0x69, 0x84, 0xC8, 0x01, 0xA3, 0x03, 0x48, 0xC8, 0x63, 0x44,
	0xD3, 0xD1, 0x1A, 0x69, 0x80, 0x26, 0xD4, 0x69, 0xA3, 0xD2, 0x51, 0x95, 0x96, 0x10, 0x81, 0x05,
	0x07, 0x1D, 0x00, 0xE8, 0x9F, 0x43, 0x8E, 0x46, 0x11, 0x50, 0xA4, 0x2C, 0x0C, 0x6A, 0xEB, 0xFA,
	0x69, 0x52, 0xE7, 0xE5, 0xC2, 0x64, 0xB9, 0x45, 0x64, 0x5A, 0xB3, 0x2C, 0xEB, 0xAC, 0x62, 0x92,
	0xBE, 0x1B, 0x35, 0x3B, 0x3E, 0xAB, 0x39, 0xB7, 0x45, 0x81, 0x44, 0x4D, 0x9D, 0x3A, 0x35, 0xD6,
	0xC9, 0x83, 0xB4, 0x9E, 0x44, 0x67, 0x6C, 0x5B, 0x1A, 0x3B, 0x52, 0x01, 0x28, 0x97, 0xCF, 0xEC,
	0x9B, 0xF7, 0xA4, 0xCE, 0xC5, 0xAB, 0x37, 0x67, 0x6F, 0x5F, 0x69, 0xFD, 0x27, 0xE1, 0x6A, 0x62,
	0x61, 0x7A, 0x4B, 0x14, 0xB0, 0xD2, 0x13, 0x06, 0x02, 0x12, 0xF2, 0x04, 0x03, 0x62, 0x13, 0x63,
	0x49, 0x82, 0x49, 0x8D, 0x21, 0x24, 0x36, 0x90, 0x36, 0x9B, 0x43, 0x6D, 0xB6, 0x21, 0x36, 0x92,
	0x4D, 0x0C, 0x18, 0xD2, 0x4D, 0xA0, 0x63, 0x13, 0x18, 0x26, 0x86, 0xC6, 0x30, 0x6D, 0x6A, 0x0C,
	0x82, 0x18, 0x98, 0x31, 0xB6, 0xD2, 0xD8, 0x1A, 0x20, 0x0C, 0xD7, 0x07, 0x00, 0x35, 0x78, 0x77,
	0x78, 0xEB, 0xC8, 0xA3, 0x8A, 0x39, 0x10, 0x48, 0x37, 0x08, 0x6F, 0x18, 0x25, 0x83, 0x84, 0xA0,
	0x4E, 0x44, 0xB0, 0x5F, 0x13, 0x71, 0x94, 0x09, 0x45, 0x8A, 0x5F, 0x92, 0x31, 0x88, 0xAA, 0x13,
	0xC8, 0xA8, 0x49, 0x22, 0xA1, 0x96, 0x91, 0x34, 0x20, 0xF0, 0x89, 0x12, 0x7B, 0xCC, 0xB2, 0x66,
	0x61, 0x43, 0xE2, 0x8C, 0xC3, 0x34, 0x94, 0x8F, 0x81, 0x9E, 0x4D, 0x55, 0x9A, 0x4E, 0x68, 0x14,
	0x2A, 0x91, 0xD7, 0x46, 0x1A, 0x46, 0x89, 0xA6, 0x74, 0x0B, 0x24, 0x48, 0xB4, 0xD4, 0x5F, 0x42,
	0x38, 0xC4, 0x4A, 0xA7, 0x35, 0x45, 0x14, 0xCC, 0x42, 0xB3, 0xEA, 0x45, 0x3A, 0xB1, 0xA8, 0x6B,
	0x29, 0x3F, 0x08, 0xB5, 0x8D, 0x47, 0x3C, 0xA5, 0x66, 0xFB, 0x9B, 0x2A, 0x50, 0xA6, 0x6E, 0x28,
	0xB5, 0xC8, 0xD2, 0x6C, 0x95, 0x8F, 0x21, 0xC0, 0x9D, 0x5D, 0xBC, 0xE4, 0xAB, 0xB4, 0x93, 0xB7,
	0xDE, 0xD5, 0x74, 0xC8, 0x40, 0x84, 0x21, 0xBE, 0x88, 0xFC, 0xA2, 0xCA, 0x62, 0x25, 0x03, 0x31,
	0x3B, 0x81, 0x8A, 0xAE, 0xEA, 0x2C, 0x9F, 0xA6, 0x52, 0x9B, 0x85, 0x51, 0xFC, 0x7B, 0x9C, 0x4B,
	0x77, 0x13, 0xA6, 0xE5, 0x2C, 0x3A, 0x4F, 0xF2, 0xD5, 0x61, 0xD7, 0x71, 0xBA, 0xAA, 0x6E, 0x72,
	0x9A, 0xD5, 0x6F, 0x0F, 0xF8, 0xBB, 0x92, 0x29, 0xC2, 0x84, 0x83, 0x78, 0x5E, 0x20, 0x30,
};
#endif // !__MORPHOS__
