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

// TextToolView.h : interface of the CTextToolView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEXTTOOLVIEW_H__5B35C5FE_29CA_4874_B1D3_14B6CBEDDF53__INCLUDED_)
#define AFX_TEXTTOOLVIEW_H__5B35C5FE_29CA_4874_B1D3_14B6CBEDDF53__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct
{
  int iStartX;
  int iEndX;
  int iStartY;
  int iEndY;
} trGlyphExtents;

class CTextToolView : public CView
{
protected: // create from serialization only
  CTextToolView();
  DECLARE_DYNCREATE(CTextToolView)

// Attributes
public:
  CTextToolDoc* GetDocument();

  CFont		*m_pFont;
  ABC		m_aWidths[256];
  int		m_iMaxTextureWidth;
  int		m_iTextureZStep;
  int		m_iFontPointSize;

  int		m_iMaxY;
  int		m_iCharacterHeight;
  trGlyphExtents	m_arGlyphExtents[256];

  int		iLogicalPixelsY;

// Operations
public:

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CTextToolView)
  public:
  virtual void OnDraw(CDC* pDC);  // overridden to draw this view
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual void OnInitialUpdate();
  protected:
  virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
  virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
  virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
  //}}AFX_VIRTUAL

// Implementation
public:
  virtual ~CTextToolView();
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
  //{{AFX_MSG(CTextToolView)
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnFontSetfont();
  void DoFontSavefontfiles(CString filename);
  afx_msg void OnFontSavefontfiles();
  afx_msg void OnBatchProcessing();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in TextToolView.cpp
inline CTextToolDoc* CTextToolView::GetDocument()
   { return (CTextToolDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTTOOLVIEW_H__5B35C5FE_29CA_4874_B1D3_14B6CBEDDF53__INCLUDED_)
