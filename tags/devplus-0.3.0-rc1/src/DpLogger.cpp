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

#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>


#include <DpLogger.h>


//------------------------------------------------------------------------------
// CJW: Initialise the logger data that will be used locally for this object.
DpLogger::DpLogger()
{
	_fp = NULL;
	_nLength = 0;
	_nMaxLength = 25000000;
	_bTimestamp = false;
	_nDepth = 0;
	_szPath = NULL;
}

//------------------------------------------------------------------------------
// CJW: Deconstructor.
DpLogger::~DpLogger()
{
	if (_fp != NULL) {
		Close();
	}
	ASSERT(_fp == NULL);
}

void DpLogger::SetTimestamp()
{
	ASSERT(_bTimestamp == false);
	_bTimestamp = true;
}


//------------------------------------------------------------------------------
// CJW: Initialise the logging system.  Basically it will move the old log files 
// 		around, and make a new log file.
bool DpLogger::Init(const char *szPath, int nDepth)
{
	ASSERT(szPath != NULL);
	ASSERT(nDepth >= 0);
	ASSERT(strlen(szPath) < 2045);
	ASSERT(_nDepth == 0 && _szPath == NULL);
	
	_szPath = (char *) szPath;
	_nDepth = nDepth;
	
	return(Init());
}


bool DpLogger::Init(void)
{
	char szOld[2048], szNew[2048];
	int nCount;
	
	ASSERT(_fp == NULL);
	ASSERT(_nDepth >= 0 && _szPath != NULL);
	
	sprintf(szOld, "%s.%d", _szPath, _nDepth);
	unlink(szOld);
	
	nCount = _nDepth;
	while(nCount > 0) {
		sprintf(szNew, "%s.%d", _szPath, nCount);
		nCount --;
		if (nCount == 0) {
			rename(_szPath, szNew);
		}
		else {
			ASSERT(nCount > 0);
			sprintf(szOld, "%s.%d", _szPath, nCount);
			rename(szOld, szNew);
		}
	}
	
	_nLength = 0;
	_fp = fopen(_szPath, "a");

	if (_fp == NULL) return false;
	else 			 return true;
}

//------------------------------------------------------------------------------
// CJW: Log the string to the file.  We will append a newline char to the end of 
// 		it, and if we are told to log the date and time, we will do that too.
void DpLogger::LogStr(char *text)
{
	int len;
	char szTime[32];
	
	struct timeval tv;
	struct timezone tz;
	struct tm tm;
	
	ASSERT(text != NULL);
	
	szTime[0] = '\0';
	if (_bTimestamp == true) {
		if(gettimeofday(&tv, &tz) == 0) {
			localtime_r(&(tv.tv_sec), &tm);
			sprintf(szTime, "%04d%02d%02d-%02d%02d%02d ", 
						tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		}
	}
	
	len = fprintf(_fp, "%s%s\n", szTime, text);
	fflush(_fp);
	
	ASSERT(len >= 0);
	ASSERT(_nLength >= 0);
	if (_nMaxLength > 0) {
		_nLength += len;
		if (_nLength >= _nMaxLength) {
			fclose(_fp);
			_fp = NULL;
		
			Init();
			ASSERT(_nLength == 0);
		}
	}
	else {
		ASSERT(_nLength == 0);
	}
}

//-----------------------------------------------------------------------------
void DpLogger::Log(char *text, ...)
{
	va_list ap;
	char szBuffer[32000];

	ASSERT(text != NULL);

	va_start(ap, text);
	vsprintf(szBuffer, text, ap);
	va_end(ap);
		
	DpLogger::LogStr(szBuffer);
}



//-----------------------------------------------------------------------------
void DpLogger::Close(void)
{
	ASSERT(_fp != NULL);
	if (_fp != NULL) {
		fclose(_fp);
		_fp = NULL;
	}
}

