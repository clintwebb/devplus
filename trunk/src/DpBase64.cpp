//-----------------------------------------------------------------------------
// DevPlus.   
//-----------------------------------------------------------------------------


/***************************************************************************
 *   Copyright (C) 2006-2007 by Hyper-Active Systems,,,                    *
 *   Copyright (C) 2003-2005 by Clinton Webb,,,                            *
 *   devplus@hyper-active.com.au                                           *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/




//-----------------------------------------------------------------------------
// CJW: If we are compiling under visual studio, then we need to include this
//      file or we will have linker problems regarding some already defined
//      functions.  This is rather annoying and really quite stupid, but that
//      is what you get I suppose.
#ifdef _MSC_VER
    #include "stdafx.h"
#endif
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>

#include <DpBase64.h>


char DpBase64::_szCodes[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
unsigned char DpBase64::_szReverse[128];
bool DpBase64::_bInit = false;


//-----------------------------------------------------------------------------
// CJW: Constructor.  Initialise the variables and states.
DpBase64::DpBase64()
{
	int i;
	
	_pEncoded = NULL;
	_pDecoded = NULL;
	
	// This information should be statically assigned as a const, but since I 
	// cant be bothered to build the array manually right now, I've written 
	// this code to do it instead.
	if (_bInit == false) {
		for (i=0; i<128; i++) { _szReverse[i] = 0; }
		for (i=0; i<64; i++)  { 
// 			printf("'%c'==%d\n", _szCodes[i], _szCodes[i]);
// 			printf("_szReverse[(unsigned int) %d] = %d\n", _szCodes[i], i);
			_szReverse[(unsigned int) _szCodes[i]] = i; 
		}
		_bInit = true;
	}
}

//-----------------------------------------------------------------------------
// CJW: Deconstructor.  Clean up the resources that this object uses.
DpBase64::~DpBase64()
{
	Clear();
}


//-----------------------------------------------------------------------------
// CJW: Clear the strings ready for another operation, or to close down the 
// 		object.
void DpBase64::Clear(void)
{
	if (_pEncoded != NULL) {
		free(_pEncoded);
		_pEncoded = NULL;
	}
	
	if (_pDecoded != NULL) {
		free(_pDecoded);
		_pDecoded = NULL;
	}
	
}

//-----------------------------------------------------------------------------
// CJW: This function should only be used if you are sure that the string 
//		pointer passed in is definately a null-terminated string.
char * DpBase64::EncodeStr(char *str)
{
	ASSERT(str != NULL);
	
	return(Encode((unsigned char *)str, strlen(str)));
}


//-----------------------------------------------------------------------------
// CJW: Encode a set of data into a base64 string.
char * DpBase64::Encode(unsigned char *str, int length)
{
	int i,j, left;
	
	ASSERT(str != NULL && length > 0);
	
	Clear();

	ASSERT(_pDecoded == NULL);
	ASSERT(_pEncoded == NULL);
	
	_pEncoded = (char *) malloc((length / 3 * 4 ) + 5);
	
	for(i=0,j=0; i<length; i+=3, j+=4) {
		left = length-i;
		if (left > 3) { left = 3; }
		EncodeBlock(&str[i], &_pEncoded[j], left);
	}
	_pEncoded[j] = '\0';
	
	return(_pEncoded);
}


//-----------------------------------------------------------------------------
// CJW: Encode a 3 byte block into a 4 byte block.
void DpBase64::EncodeBlock(unsigned char *in, char *out, int len) 
{
	ASSERT(in != NULL && out != NULL && len > 0);
	
	out[0] = _szCodes[ in[0] >> 2 ];
	out[1] = _szCodes[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
	if (len > 1) { out[2] = _szCodes[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] ; }
	else 		 { out[2] = '='; }
	if (len > 2) { out[3] = _szCodes[ in[2] & 0x3f ]; }
	else 		 { out[3] = '='; }
}


//-----------------------------------------------------------------------------
// CJW: Decode a 4 byte base64 block into a 3 byte binary block.
unsigned char * DpBase64::Decode(char *str, int *length)
{
	int len;
	int i,j, used;
	
	ASSERT(str != NULL && length != NULL);
	
	Clear();

	ASSERT(_pDecoded == NULL);
	ASSERT(_pEncoded == NULL);
	
	len = strlen(str);
	ASSERT(len > 0);
	_pDecoded = (unsigned char *) malloc(len+1);
	
	length[0] = 0;
	for(i=0,j=0; j<len; i+=3, j+=4) {
		while(str[j] == '\n' || str[j] == '\r') {
			j++;
		}
		ASSERT(j < len);
		DecodeBlock(&str[j], &_pDecoded[i], &used);
		ASSERT(used > 0 && used <= 3);
		length[0] += used;
		ASSERT(length[0] < len);
	}
	_pDecoded[length[0]] = '\0';
	
	return(_pDecoded);
	
}


//-----------------------------------------------------------------------------
// CJW: Decode a 4 byte base64 block into a 3 byte binary block.
void DpBase64::DecodeBlock(char *in, unsigned char *out, int *len) 
{
	unsigned char ch;
	
	ASSERT(in != NULL && out != NULL && len != NULL);
	ASSERT(in[0] != '=' && in[0] != '\0');
	ASSERT(in[1] != '=' && in[1] != '\0');

	len[0] = 1;
	
// 	printf("in[0] == '%c' (%d)\n", in[0], in[0]);
// 	printf("_szReverse[in[0]] == (%d)\n", _szReverse[in[0]]);
	
	out[0] = (_szReverse[(int)in[0]]) << 2;
// 	printf("out[0] == '%c' (%d)\n", out[0], out[0]);
	
	ch = _szReverse[(int)in[1]];
// 	printf("ch = '%c' (%d)\n", ch, ch);
	out[0] |= ((ch >> 4) & 0x03);
// 	printf("ch>>6 = '%c' (%d)\n", ch>>4, ch>>4);
// 	printf("out[0] == '%c' (%d)\n", out[0], out[0]);
	if (in[2] != '=') {
		len[0] ++;
		out[1] = ((ch & 0x0f) << 4);
		ch = _szReverse[(int)in[2]];
		out[1] |= (ch >> 2);
		
		if (in[3] != '=') {
			len[0] ++;
			out[2] = (ch & 0x03) << 6;
			out[2] |= _szReverse[(int)in[3]];
		}
	}
}



