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

#include <DevPlus.h>
#include <DpWinService.h>




//------------------------------------------------------------------------------
// CJW: Allocate our static data.
void *DpWinService::_pThis = NULL;



//------------------------------------------------------------------------------
// CJW: Constructor.  Initialise our structures.
DpWinService::DpWinService()
{
    _pThis = this;
    _szName = NULL;
}



//------------------------------------------------------------------------------
// CJW: Deconstructor.  Clean up.
DpWinService::~DpWinService()
{
}


//------------------------------------------------------------------------------
// CJW: Set the name of the service.  Required.
void DpWinService::SetServiceName(const char *name)
{
    ASSERT(name != NULL);
    ASSERT(_szName == NULL);

    _szName = name;
}


//------------------------------------------------------------------------------
// CJW: This function is where the service code is being done.  This function
//      should be over-ridden and handled in the child-class.
void DpWinService::Run(void)
{
    _xLock.Lock();
    while(_bRun != false) {
        _xLock.Unlock();
        Sleep(3000);        // sleep for 3 seconds.
        _xLock.Lock();
    }
    _xLock.Unlock();
}


//------------------------------------------------------------------------------
// CJW: Install the service.
bool DpWinService::Install(void)
{
    bool bOK = false;
    char szFilePath[_MAX_PATH];

    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager(NULL, // local machine
                                     NULL, // ServicesActive database
                                     SC_MANAGER_ALL_ACCESS); // full access
    if (hSCM) {

        // Get the executable file path
        ::GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));

        // Create the service
        SC_HANDLE hService = ::CreateService(hSCM,
                                             _szServiceName,
                                             _szServiceName,
                                             SERVICE_ALL_ACCESS,
                                             SERVICE_WIN32_OWN_PROCESS,
                                             SERVICE_DEMAND_START,        // start condition
                                             SERVICE_ERROR_NORMAL,
                                             szFilePath,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL);

        if (hService) {
            bOK = true;
            ::CloseServiceHandle(hService);
        }

        ::CloseServiceHandle(hSCM);
    }

    return bOK;
}



