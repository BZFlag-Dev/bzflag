/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// TextTool.h : main header file for the TEXTTOOL application
//

#if !defined(AFX_TEXTTOOL_H__3A2264FC_6BE5_4B22_A1D4_8CE11C29F90B__INCLUDED_)
	#define AFX_TEXTTOOL_H__3A2264FC_6BE5_4B22_A1D4_8CE11C29F90B__INCLUDED_

	#if _MSC_VER > 1000
		#pragma once
	#endif // _MSC_VER > 1000

	#ifndef __AFXWIN_H__
		#error include 'stdafx.h' before including this file for PCH
	#endif 

	#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CTextToolApp:
// See TextTool.cpp for the implementation of this class
//

class CTextToolApp: public CWinApp
{
public:
	CTextToolApp();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextToolApp)
public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	// Implementation
	//{{AFX_MSG(CTextToolApp)
	afx_msg void OnAppAbout();
	// NOTE - the ClassWizard will add and remove member functions here.
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

// This is ugly
extern CStatusBar *sbar;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTTOOL_H__3A2264FC_6BE5_4B22_A1D4_8CE11C29F90B__INCLUDED_)
