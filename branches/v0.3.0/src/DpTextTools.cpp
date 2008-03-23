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

#include <DpTextTools.h>


//------------------------------------------------------------------------------
// CJW: Constructor for the DpTextTools class.
DpTextTools::DpTextTools()
{
    _pData = NULL;
    _nItems = 0;

    _nCurrentLine = 0;
    _nCurrentChar = 0;

    _pWordArray = NULL;
}

//------------------------------------------------------------------------------
// CJW: Deconstructor.
DpTextTools::~DpTextTools()
{
    ClearWordArray();

    if (_pData != NULL) {
        ASSERT(_nItems != 0);

        while(_nItems > 0) {
            _nItems--;
            if (_pData[_nItems].data != NULL) {
                free(_pData[_nItems].data);
            }
        }

        free(_pData);
    }
    ASSERT(_nItems == 0);
}

//------------------------------------------------------------------------------
// CJW: Load the text from a file.   This function will get the data, and then
//      pass it to a ParseLines() function which will seperate the text into
//      individual lines.
//
//  TODO:   If the file is really big, we should probably process it in chunks.
bool DpTextTools::LoadFromFile(char *file)
{
    bool ok = false;
    FILE *fp;
    char *buffer = NULL;    // first we read the data into the buffer, then we will
    long length = 0;        // the number of chars in our buffer.
    int result;

    ASSERT(file != NULL);
    ASSERT(_pData == NULL);
    ASSERT(_nItems == 0);

    fp = fopen(file, "r");
    if (fp != NULL) {

        buffer = (char *) malloc(1024);
        ASSERT(buffer != NULL);

        result = fread(buffer, 1, 1024, fp);
        while (result > 0) {
            length += result;

            buffer = (char *) realloc(buffer, length + 1024);
            ASSERT(buffer != NULL);
            result = fread(&buffer[length], 1, 1024, fp);
        }

        fclose(fp);

        // Now we have the entire contents of the file in our buffer, we need 
        // to go thru it and add each line to an array.
        ok = Load(buffer);

        if (buffer != NULL) {
            free(buffer);
        }
    }

    return(ok);
}

//------------------------------------------------------------------------------
// CJW: Load the text into our line array.
bool DpTextTools::Load(char *buffer)
{
    bool ok = false;
    char *str;
    long count;
    char ch;
    int len;
    int length;

    ASSERT(buffer != NULL);

    // start the first line.
    _pData = (DpTextLine_t *) malloc(sizeof(DpTextLine_t));
    ASSERT(_pData != NULL);
    if (_pData != NULL) {
        str = (char *) malloc(TEXT_MAX_LINE_LENGTH);
        len = 0;

        length = strlen(buffer);
        for(count=0; count<length; count++) {
            ch = buffer[count];

            if (len >= TEXT_MAX_LINE_LENGTH) {
                str = (char *) realloc(str, len + 2);
            }

            if (ch == '\n') {
                str[len] = '\0';

                _pData[_nItems].data = (char *) realloc(str, len+1);
                _pData[_nItems].length = len;
                _nItems++;

                _pData = (DpTextLine_t *) realloc(_pData, sizeof(DpTextLine_t) * (_nItems+1));
                ASSERT(_pData != NULL);

                str = (char *) malloc(TEXT_MAX_LINE_LENGTH);
                len = 0;
            }
            else {
                str[len] = ch;
                len++;
            }
        }

        if (len > 0) {
            str[len] = '\0';
            _pData[_nItems].data = (char *) realloc(str, len+1);
            _pData[_nItems].length = len;
            _nItems++;
        }
        else {
            free(str);
            _nItems--;
            ASSERT(_nItems >= 0);
        }

        ok = true;
    }

    return(ok);
}



//------------------------------------------------------------------------------
// CJW: Given some text, we will find the line that BEGINS with this particular
//      text, starting from the current line.  If you wish to search for
//      particular text ANYWHERE in a line, then the general FindText function
//      would be more appropriate.  However, if you know the text you are
//      looking for is at the begining of the line, then this function will
//      perform much faster.
int DpTextTools::FindLine(char *txt)
{
    int nLine = -1;
    int count;
    int len;
    int glen;
    
    ASSERT(txt != NULL);
    ASSERT(_pData != NULL);

    glen = strlen(txt);

    count = _nCurrentLine;
    while(nLine == -1 && count < _nItems) {
        len = _pData[count].length;
        if (glen < len) { len = glen; }

        if (strncmp(txt, _pData[count].data, glen) == 0) {
            nLine = count;
            _nCurrentLine = nLine;
            _nCurrentChar = 0;
            count = _nItems;
        }
    
        count++;
    }

    return(nLine);
}

//------------------------------------------------------------------------------
// CJW: Move the 'cursor' to the begining of the next line.
int DpTextTools::MoveNextLine(void)
{
    int nLine;

    nLine = _nCurrentLine + 1;
    if (nLine < _nItems) {
        _nCurrentChar = 0;
        _nCurrentLine = nLine;
    }
    else {
        nLine = -1;
    }

    return(nLine);
}

//------------------------------------------------------------------------------
// CJW: This function is still quite simple, but is a little more complicated
//      than what we have done so far.  This one will delete all lines before
//      the current one.  This means, that we need to make sure that we dont
//      clobber ourselves if people start this function on the first line.   We
//      also need to be very careful about freeing all the memory that is no
//      longer needed.  To accomplish this, we will need several line counters,
//      and will need to process each line that we have.  some we will need to
//      move, others we will need to delete.
int DpTextTools::TrimBefore(void)
{
    int nLine = -1;
    int a, b, c;

    if (_nCurrentLine > 0) {

        a = 0;
        b = _nCurrentLine;

        for (c=0; c<_nItems; c++) {
            if (_pData[c].data != NULL) {
                free(_pData[c].data);
                _pData[c].data = NULL;
            }

            if (b < _nItems) {
                _pData[a].data = _pData[b].data;
                _pData[a].length = _pData[b].length;

                _pData[b].data = NULL;
                a++;
                b++;
            }
        }

        _nCurrentLine = 0;
        _nItems = a;
        nLine = 0;

        _pData = (DpTextLine_t *) realloc(_pData, sizeof(DpTextLine_t) * _nItems);
        ASSERT(_pData != NULL);
    }

    return(nLine);
}

//------------------------------------------------------------------------------
// CJW: Starting from the current line, delete everything to the end of the
//      list.  Move the current line marker to the previous line.
int DpTextTools::DeleteToEnd(void)
{
    int count;

    for (count=_nCurrentLine; count<_nItems; count++) {
        if (_pData[count].data != NULL) {
            free(_pData[count].data);
            _pData[count].data = NULL;
            _pData[count].length = 0;
        }
    }

    _nItems = _nCurrentLine;
    _pData = (DpTextLine_t *) realloc(_pData, sizeof(DpTextLine_t) * _nItems);
    ASSERT(_pData != NULL);

    if (_nCurrentLine > 0) {
        _nCurrentLine--;
    }

    return(_nCurrentLine);
}

//------------------------------------------------------------------------------
// CJW: Move the cursor to the first line in the array.  Also move to the char
//      of the line.
int DpTextTools::MoveFirstLine(void)
{
    _nCurrentLine = 0;
    _nCurrentChar = 0;
    return(0);
}

//------------------------------------------------------------------------------
// CJW: On the current line, search for a particular string, and insert a new
//      line into the text array whenever it is found.
int DpTextTools::InsertLineOnStr(char *str)
{
    char *text;     // our original line text.
    int length;     // the length of our original line.
    int a, b;       // general purpose counters/
    int len;        // length of our trigger string.
    int count;

    ASSERT(str != NULL);
    ASSERT(_nCurrentLine < _nItems);
    ASSERT(_pData != NULL);
    ASSERT(_nItems > 0);

    len = strlen(str);

    length = _pData[_nCurrentLine].length;
    text = (char *) malloc(length + 1);
    ASSERT(text != NULL);
    if (text != NULL) {
        strcpy(text, _pData[_nCurrentLine].data);
    
        count = 0;
        b = _pData[_nCurrentLine].length - len;
        for (a = _nCurrentChar; a<b; a++) {
            if (strncmp(str, &text[a], len) == 0) {
                // we found our string.. insert the new line, reset our counts, and then skip the word we found
                _nCurrentChar = count;
                InsertNewLine();
                a += len;
                count = len+1;
            }
            else {
                count++;
            }
        }

        free(text);
    }

    return(_nCurrentLine);
}

//------------------------------------------------------------------------------
// CJW: Insert a new line.  If there is text following cursor in the line, then
//      it will truncate the current one, and move the text to the newly created
//      new line.
int DpTextTools::InsertNewLine(void)
{
    int a;

    ASSERT(_nCurrentLine < _nItems);
    ASSERT(_pData != NULL);

    // increase the number of lines;
    _pData = (DpTextLine_t *) realloc(_pData, sizeof(DpTextLine_t) * (_nItems+1));
    ASSERT(_pData != NULL);

    // now move all the line data down by one.
    for(a=_nItems; a>(_nCurrentLine+1); a--) {
        _pData[a].data = _pData[a-1].data;
        _pData[a].length = _pData[a-1].length;
    }
    _nItems++;

    _pData[_nCurrentLine+1].data = (char *) malloc((_pData[_nCurrentLine].length - _nCurrentChar) + 2);
    ASSERT(_pData[_nCurrentLine+1].data != NULL);
    
    _pData[_nCurrentLine+1].length = _pData[_nCurrentLine].length - _nCurrentChar;
    strncpy(_pData[_nCurrentLine+1].data, &_pData[_nCurrentLine].data[_nCurrentChar], _pData[_nCurrentLine+1].length);
    _pData[_nCurrentLine+1].data[_pData[_nCurrentLine+1].length] = '\0';

    // now we need to trim the original and then move the current line and cursor to the new line.
    _pData[_nCurrentLine].data = (char *) realloc(_pData[_nCurrentLine].data, _nCurrentChar + 2);
    ASSERT(_pData[_nCurrentLine].data != NULL);

    _pData[_nCurrentLine].length = _nCurrentChar;
    _pData[_nCurrentLine].data[_nCurrentChar] = '\0';

    _nCurrentLine ++;
    _nCurrentChar = 0;

    return(_nCurrentLine);
}

//------------------------------------------------------------------------------
// CJW: On the current line, this function will remove all the HTML tags.   If
//      the spaces parameter is true, then it will ensure there is a space
//      between all text left over after html has been trimmed out.  For
//      example, with spaces set to true, "<b><i>title</i></b>fred" would
//      result in "title fred".  With spaces set to false, it would come out
//      as "titlefred".
void DpTextTools::RemoveHtmlOnLine(bool spaces)
{
    char *text;
    int length, len;
    int vv;
    int count;
    char ch;
    bool last = false;      // last real text had a space?

    ASSERT(_nCurrentLine < _nItems);
    ASSERT(_pData != NULL);

    length = 0;
    vv = 0;

    len = _pData[_nCurrentLine].length;
    text = (char *) malloc(len + 1);
    ASSERT(text != NULL);
    if (text != NULL) {

        for(count=0; count < len; count++) {
            ch = _pData[_nCurrentLine].data[count];

            if (ch == '<') {
                vv++;
            }
            else if (ch == '>') {
                vv--;
                if (vv == 0 && spaces==true) {
                    last = true;
                }
            }
            else {
                if (vv == 0) {
                    // do we add a space?
                    if (spaces == true) {
                        if (length > 0) {
                            if (last == true) {
                                if (text[length-1] != ' ') {
                                    text[length] = ' ';
                                    length++;
                                }
                            }
                        }
                        last = false;
                    }

                    text[length] = ch;
                    length++;
                }
            }
        }

        // realloc text.
        text[length] = '\0';
        text = (char *) realloc(text, length+1);

        // free original
        free(_pData[_nCurrentLine].data);

        // put text in current line.
        _pData[_nCurrentLine].data = text;
        _pData[_nCurrentLine].length = length;
    }
}

//------------------------------------------------------------------------------
// CJW: Return a char ptr array of the words that are in the line.  We will be
//      returning pointers to copies of the actual words, so modifying the
//      values inside this array will not change the actual text.  However, it
//      is important to note that the user is not responsible for releasing the
//      memory used by the elements in this array, but IS responsible for
//      releasing the array itself.  However, only one of these arrays can be
//      used at a time, because an internal pointer is used to maintain it, and
//      free the strings when this function either goes out of scope, or
//      another one of these function calls is made.
char **DpTextTools::GetWordArray(void)
{
    char **pta = NULL;
    int items = 0;
    int len;
    int count;
    bool next = true;   // true means that the next char is the begining of a word if it is not a space.
    char ch;

    ASSERT(_nCurrentLine < _nItems);
    ASSERT(_pData != NULL);

    // clear the word array if we already have one.
    ClearWordArray();

    // copy the current line to the word array pointer.
    len = _pData[_nCurrentLine].length;
    _pWordArray = (char *) malloc(len + 1);
    ASSERT(_pWordArray != NULL);
    if (_pWordArray != NULL) {
    
        strncpy(_pWordArray, _pData[_nCurrentLine].data, len+1);

        for(count=0; count<len; count++) {
            ch = _pWordArray[count];
            if (ch == ' ' || ch == '\t' || ch == ',') {
                _pWordArray[count] = '\0';
                next = true;
            }
            else {
                if (next == true) {
                    pta = (char **) realloc(pta, sizeof(char *) * (items + 1));
                    ASSERT(pta != NULL);
                    pta[items] = &_pWordArray[count];
                    items++;
                    next = false;
                }
            }
        }

        pta = (char **) realloc(pta, sizeof(char *) * (items + 1));
        ASSERT(pta != NULL);
        pta[items] = NULL;
        items++;
    }

    return(pta);
    
}

//------------------------------------------------------------------------------
// CJW: If a word array was generated on a line, this function will release all
//      the memory that was allocated to hold all the words.
void DpTextTools::ClearWordArray(void)
{
    if (_pWordArray != NULL) {
        free(_pWordArray);
        _pWordArray = NULL;
    }
}


//------------------------------------------------------------------------------
// CJW: Sometimes there are particular words that we want to delete on a line.
//      This function will do that.   When finished, the cursor will be moved to
//      the begining of the line.  Will return the number of times the word was
//      removed from the line.
int DpTextTools::DeleteFromLine(char *str)
{
    int words = 0;

    char *text;
    int length, len;
    int count;
    int slen;

    ASSERT(str != NULL);

    ASSERT(_nCurrentLine < _nItems);
    ASSERT(_pData != NULL);

    slen = strlen(str);
    ASSERT(slen > 0);

    length = 0;
    text = (char *) malloc(_pData[_nCurrentLine].length + 1);
    ASSERT(text != NULL);
    if (text != NULL) {

        len = _pData[_nCurrentLine].length - slen;
        for(count=0; count < len; count++) {
            if (strncmp(str, &_pData[_nCurrentLine].data[count], slen) == 0) {
                count += (slen-1);  // -1 because the for will increment one also.
                words++;
            }
            else {
                text[length] = _pData[_nCurrentLine].data[count];
                length++;
            }
        }

        // realloc text.
        text[length] = '\0';
        text = (char *) realloc(text, length+1);

        // free original
        free(_pData[_nCurrentLine].data);

        // put text in current line.
        _pData[_nCurrentLine].data = text;
        _pData[_nCurrentLine].length = length;
    }

    _nCurrentChar = 0;

    return(words);
}




