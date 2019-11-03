// dlldrvobj.cpp : Defines the initialization routines for the DLL.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2015 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// 
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
//
// Linux
// =====
// device1 = xapdrv,127.0.0.1;3639,txt,/usr/local/lib/xapdrv.so,64,64,1
//
// WIN32
// =====
// device1 = xapdrv,127.0.0.1;3639,d:\winnr\system32\xapdrv.dll,64,64,1

#include "stdio.h"
#include "stdlib.h"
#include "dlldrvobj.h"
#include "xapobj.h"

#ifdef WIN32

#else

void _init() __attribute__((constructor));
void _fini() __attribute__((destructor));

void _init() {printf("initializing\n");}
void _fini() {printf("finishing\n");}

#endif

/////////////////////////////////////////////////////////////////////////////
// CDllDrvObj

////////////////////////////////////////////////////////////////////////////
// CDllDrvObj construction

CDllDrvObj::CDllDrvObj()
{
	m_instanceCounter = 0;
#ifdef WIN32
	m_objMutex = CreateMutex( NULL, true, TEXT("__CANAL_XAPDRV_MUTEX__") );
#else
	pthread_mutex_init( &m_objMutex, NULL );
#endif

	// Init the driver array
	for ( int i = 0; i<CANAL_XAP_DRIVER_MAX_OPEN; i++ ) {
		m_drvObjArray[ i ] = NULL;
	}

	UNLOCK_MUTEX( m_objMutex );
}


CDllDrvObj::~CDllDrvObj()
{
	LOCK_MUTEX( m_objMutex );
	
	for ( int i = 0; i<CANAL_XAP_DRIVER_MAX_OPEN; i++ ) {
		
		if ( NULL != m_drvObjArray[ i ] ) {
			
			CXAPObj *pdrvObj =  getDriverObject( i );
			if ( NULL != pdrvObj ) { 
				pdrvObj->close();	
				delete m_drvObjArray[ i ];
				m_drvObjArray[ i ] = NULL; 
			}
		}
	}

	UNLOCK_MUTEX( m_objMutex );

#ifdef WIN32
	CloseHandle( m_objMutex );
#else	
	pthread_mutex_destroy( &m_objMutex );
#endif
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDllDrvObjgerdllApp object

//CDllDrvObj theApp;



///////////////////////////////////////////////////////////////////////////////
// addDriverObject
//

long CDllDrvObj::addDriverObject( CXAPObj *pdrvObj )
{
	long h = 0;

	LOCK_MUTEX( m_objMutex );
	for ( int i=0; i<CANAL_XAP_DRIVER_MAX_OPEN; i++ ) {
	
		if ( NULL == m_drvObjArray[ i ] ) {
		
			m_drvObjArray[ i ] = pdrvObj;	
			h = i + 1681; 
			break;

		}

	}

	UNLOCK_MUTEX( m_objMutex );

	return h;
}


///////////////////////////////////////////////////////////////////////////////
// getDriverObject
//

CXAPObj * CDllDrvObj::getDriverObject( long h )
{
	long idx = h - 1681;

	// Check if valid handle
	if ( idx < 0 ) return NULL;
	if ( idx >= CANAL_XAP_DRIVER_MAX_OPEN ) return NULL;
	return m_drvObjArray[ idx ];
}


///////////////////////////////////////////////////////////////////////////////
// removeDriverObject
//

void CDllDrvObj::removeDriverObject( long h )
{
	long idx = h - 1681;

	// Check if valid handle
	if ( idx < 0 ) return;
	if ( idx >= CANAL_XAP_DRIVER_MAX_OPEN ) return;

	LOCK_MUTEX( m_objMutex );
	if ( NULL != m_drvObjArray[ idx ] ) delete m_drvObjArray[ idx ];
	m_drvObjArray[ idx ] = NULL;
	UNLOCK_MUTEX( m_objMutex );
}

///////////////////////////////////////////////////////////////////////////////
// InitInstance

BOOL CDllDrvObj::InitInstance() 
{
	m_instanceCounter++;
	return TRUE;
}


 
