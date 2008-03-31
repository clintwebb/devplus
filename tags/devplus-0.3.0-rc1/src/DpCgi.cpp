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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <DpCgi.h>


//-----------------------------------------------------------------------------
// CJW: Constructor.  We must initialise our data first.   Since we will be 
//		using an array of form data, we need to start if off in a cleared 
//		state.
DpCgiFormData::DpCgiFormData()
{
	_pItemList = NULL;
	_nItems = 0;
}





//-----------------------------------------------------------------------------
// CJW:	Deconstructor.  If we formulated any data, or reserved any resources, we need to clean them up.  Specifically, the 
DpCgiFormData::~DpCgiFormData()
{
	if(_pItemList != NULL) {
		while(_nItems > 0) {
			_nItems--;
			if (_pItemList[_nItems].name)  free(_pItemList[_nItems].name);
			if (_pItemList[_nItems].value) free(_pItemList[_nItems].value);
		}

		free(_pItemList);
		_pItemList = NULL;
		_nItems = 0;
	}
}


//-----------------------------------------------------------------------------
// CJW: Process the environment and parameters for form data that was passed to 
//		the script.   Keep in mind that there might be some binary files that 
//		were passed.  These will need to be stored in this object some how.
//
//		The form data will be stored in an array that is used as a hash.  All 
//		references to the form data will likely be thru a named entity.  These 
//		can be accessed by using an [] operator.
void DpCgiFormData::Process(void)
{
	char *method;
	char *content;
	int index;		// index into our current field we are populating.
	char *query = NULL;
	int tmp=0;
	
	method = getenv("REQUEST_METHOD");
	if(method != NULL) {
		if(strcmp(method, "POST")  == 0) {
			content = getenv ("CONTENT_LENGTH");
			if (content) {
				index = atoi (content);
				query = (char *) malloc (index+1);
				query[0] = '\0';
				tmp = 0;
				while(tmp < index) {
					query[tmp++] = fgetc(stdin);
				}
				query[tmp++] = '\0';
			}
		}
		else {
			// Get the query string from the environment variable. 
			query = getenv("QUERY_STRING");
		}
	}

	ProcessData(query);

	// free the query string if we allocated memory for it.
	if (tmp > 0) {
		free(query);
	}
}

//-----------------------------------------------------------------------------
// CJW: This function will take the string and break apart all the form data 
// 		into name/value pairs.
void DpCgiFormData::ProcessData(char *query)
{
	int index;		// index into our current field we are populating.
	char *key;
	char *value;
	int hex;
	enum {
		start,
		inkey,
		assign,
		invalue,
	} status = start;
	int n;
	int len;
	
	ASSERT(query != NULL);
	
	// now we need to go thru that query string, and pull out the data pairs.
	if (query != NULL) {
		len = strlen(query);
		for(n=0; n<len; n++) {
			
			switch(status) {
				case start:
					key = (char*) malloc(255);
					status = inkey;
					index = 0;
					// nobreak

				case inkey:
					if(query[n] == '=') {
						key[index] = '\0';
						index++;
						key = (char *) realloc(key, index);
						status = assign;
					}
					else {
						// ** need to parse special characters.
						if(query[n] == '+') {
							key[index] = ' ';
						}
						else if(query[n] == '%') {
							hex = 0;
							hex = 0;
							hex = GetHex(query[n+1]);
							hex = hex * 16;
							hex += GetHex(query[n+2]);
							key[index] = hex;
							n+=2;
						}
						else {
							key[index] = query[n];
						}
						index++;
						if(index >= 255) {
							key = (char *) realloc(key, index+2);
						}
					}
					break;

				case assign:
					value = (char *) malloc(255);
					index = 0;
					status = invalue;
					// nobreak

				case invalue:
					if(query[n] == '&') {
						value[index] = '\0';
						index++;
						value = (char *) realloc(value, index);
			
						AddItem(key, value);
						status = start;
					}
					else {
						if(query[n] == '+') {
							value[index] = ' ';
						}
						else if(query[n] == '%') {
							hex = 0;
							hex = GetHex(query[n+1]);
							hex = hex * 16;
							hex += GetHex(query[n+2]);
							value[index] = hex;
							n+=2;
						}
						else {
							value[index] = query[n];
						}

						index++;
						if(index >= 255) {
							value = (char *) realloc(value, index+2);
						}
					}
					break;
					
				default:
					break;
			}
		}	

		if (status == invalue) {
			value[index] = '\0';
			index++;
			value = (char *) realloc(value, index);

			AddItem(key, value);
		}
	}	
}


//-----------------------------------------------------------------------------
// CJW: Add a form element to the form list.  We will simply add the pointers, 
//		so we need to make sure that the data is in heap memory and is not 
//		going to be freed.  We will free the memory when this object goes out 
//		of scope.
void DpCgiFormData::AddItem(char *key, char *value)
{
	ASSERT(key != NULL && value != NULL);
	ASSERT((_pItemList != NULL && _nItems > 0) || (_pItemList == NULL && _nItems == 0))
	
	_pItemList = (_DpCgiFormItem *) realloc(_pItemList, sizeof(_DpCgiFormItem)*(_nItems+1));
	if (_pItemList) {
		_pItemList[_nItems].name = key;
		_pItemList[_nItems].value = value;
		_nItems++;
	}
	else {
		_nItems = 0;
	}
}


//-----------------------------------------------------------------------------
// return the value of a passed parameter.
char * DpCgiFormData::Get(char *key)
{
	char *val = NULL;
	unsigned int n;
	
	ASSERT(key != NULL);
	ASSERT((_pItemList != NULL && _nItems > 0) || (_pItemList == NULL && _nItems == 0));

	for(n=0; n<_nItems; n++) {
		if(strcmp(_pItemList[n].name, key) == 0) {
			val = _pItemList[n].value;
			n=_nItems;
		}
	}
	return(val);
}


//-----------------------------------------------------------------------------
// using the [] operator... return the value of a passed parameter.
char * DpCgiFormData::operator()(char* key)
{
	ASSERT(key != NULL);
	return(Get(key));
}



//-----------------------------------------------------------------------------
// CJW: passing in a character which is either 0-9, a-f or A-F, we will return 
//		the numberical value of the character entered.
unsigned int DpCgiFormData::GetHex(char ch)
{
	unsigned int val;

	if (ch >= '0' && ch <= '9')
		val = ch - '0';
	else if (ch >= 'A' && ch <= 'F')
		val = 10 + (ch - 'A');
	else if (ch >= 'a' && ch <= 'f')
		val = 10 + (ch - 'a');

	return(val);
}


// ##########

//-----------------------------------------------------------------------------
// CJW: Constructor.  Set our flags and other variables that are used to keep 
// 		track of the state of things and help trap programmer errors.
DpCgi::DpCgi()
{
	_bSetContentType = false;
}

//-----------------------------------------------------------------------------
// CJW: Deconstructor.  Clean up anything we created along the way.
DpCgi::~DpCgi()
{
}

//-----------------------------------------------------------------------------
// CJW: When providing output as a CGI, we must tell the receiving system what 
// kind of data it is that we are returning.  Normally you use 'text/html' to 
// indicate that the result is html.
void DpCgi::ContentType(char *tt) 
{
	ASSERT(tt != NULL);
	ASSERT(_bSetContentType == false);
	printf("Content-type: %s\n\n", tt);
	_bSetContentType = true;
}



