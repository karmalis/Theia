
// GUIApp.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CGUIApp:
// See GUIApp.cpp for the implementation of this class
//

class CGUIApp : public CWinApp
{
public:
	CGUIApp();

// Overrides
public:
	virtual BOOL InitInstance();

	// Loaded Connector Module
	HMODULE connectorMod;

// Implementation

	DECLARE_MESSAGE_MAP()

	
};

extern CGUIApp theApp;