// TextToolView.cpp : implementation of the CTextToolView class
//

#include "stdafx.h"
#include "TextTool.h"

// for png compression
#include "../../src/zlib/zconf.h"
#include "../../src/zlib/zlib.h"

// for htonl
#include "winsock2.h"

// interface headers
#include "TextToolDoc.h"
#include "TextToolView.h"

// png filewriting
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextToolView

IMPLEMENT_DYNCREATE(CTextToolView, CView)

BEGIN_MESSAGE_MAP(CTextToolView, CView)
	//{{AFX_MSG_MAP(CTextToolView)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_FONT_SETFONT, OnFontSetfont)
	ON_COMMAND(ID_FONT_SAVEFONTFILES, OnFontSavefontfiles)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextToolView construction/destruction

CTextToolView::CTextToolView()
{
	// TODO: add construction code here
	iLogicalPixelsY = -1;
}

CTextToolView::~CTextToolView()
{
}

BOOL CTextToolView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	m_iMaxTextureWidth = 512;
	m_iFontPointSize = -43;
	m_iTextureZStep = 16;

	m_pFont = new CFont;
	m_pFont->CreateFont(m_iFontPointSize,0,0,0,FW_NORMAL,FALSE,FALSE,0,
					  ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
					  DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,"Arial");

	LOGFONT		rLogFont;
	m_pFont->GetLogFont(&rLogFont);

	m_iFontPointSize = -((rLogFont.lfHeight*72)/iLogicalPixelsY);

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CTextToolView drawing

void CTextToolView::OnDraw(CDC* pDC)
{
	CTextToolDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here

	if (iLogicalPixelsY == -1)
	{
		iLogicalPixelsY = pDC->GetDeviceCaps(LOGPIXELSY);

		LOGFONT		rLogFont;
		m_pFont->GetLogFont(&rLogFont);

		m_iFontPointSize = -((rLogFont.lfHeight*72)/iLogicalPixelsY);
	}
	
	CBrush	brush(RGB(0,0,0));

	pDC->SetBkMode(OPAQUE);
	RECT	rect;
	GetWindowRect(&rect);
//	pDC->GetBoundsRect(&rect,0);
	pDC->FillRect(&rect,&brush);

	// just in case
	rect.top = 0;
	rect.bottom = 1024;
	rect.left = 0;
	rect.right = 1024;
	pDC->FillRect(&rect,&brush);

	pDC->SetBkMode(TRANSPARENT );
	pDC->SetTextColor(RGB(255,255,255));
	pDC->SetBkColor(RGB(0,0,0));
	pDC->SelectObject(m_pFont);

	CPen	pen(PS_SOLID,1,RGB(128,128,128));
	CPen	pen2(PS_SOLID,1,RGB(64,64,255));
	pDC->SelectObject(pen);

	CString		szString = " !\"#$%'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ{\\]^_`abcdefghijklmnopqrstuvwxyz{|}";
	pDC->GetCharABCWidths(' ','}',m_aWidths);

	LOGFONT			rLogFont;

	m_pFont->GetLogFont(&rLogFont);
	
	CSize size = pDC->GetTextExtent(szString);

	m_iCharacterHeight = abs(rLogFont.lfHeight);
	m_iTextureZStep = size.cy; //-rLogFont.lfHeight+2;

	int iXPos = 0;
	int	iYPos = 0;

	int iXCount = 0;
	for (int iChar = ' '; iChar <= '}';iChar++)
	{
		int iThisItem = iChar-' ';

		int iThisPos = iXPos - m_aWidths[iThisItem].abcA;

		int iEndPos = iXCount + m_iTextureZStep;//m_aWidths[iChar-' '].abcB;

		if (iEndPos >= m_iMaxTextureWidth)
		{
			iXPos = 0;
			iXCount = 0;
			iThisPos = iXPos - m_aWidths[iThisItem].abcA;
			iEndPos = iXCount + m_iTextureZStep;//m_aWidths[iChar-' '].abcB;
			iYPos += m_iTextureZStep;
			
	//		pDC->MoveTo(0,iYPos);
	//		pDC->LineTo(m_iMaxTextureWidth,iYPos);
		}
		
		CString string = (char)iChar;

		pDC->TextOut(iThisPos,iYPos,string);

//		pDC->MoveTo(iXCount,iYPos);
//		pDC->LineTo(iXCount,iYPos+m_iTextureZStep);

//		pDC->SelectObject(pen2);
//		pDC->MoveTo(iXCount+m_aWidths[iThisItem].abcB,iYPos+2);
//		pDC->LineTo(iXCount+m_aWidths[iThisItem].abcB,iYPos+m_iTextureZStep-2);

		m_arGlyphExtents[iThisItem].iStartX = iXCount;
		m_arGlyphExtents[iThisItem].iEndX = iXCount+m_aWidths[iThisItem].abcB;
		m_arGlyphExtents[iThisItem].iStartY = iYPos;
		m_arGlyphExtents[iThisItem].iEndY = iYPos+m_iTextureZStep;

		pDC->SelectObject(pen);

		iXPos = iEndPos;
		iXCount += m_iTextureZStep;
	}
//	pDC->MoveTo(0,iYPos+m_iTextureZStep);
//	pDC->LineTo(m_iMaxTextureWidth,iYPos+m_iTextureZStep);

	m_iMaxY = iYPos +m_iTextureZStep;
	pDC->SelectStockObject(SYSTEM_FONT);
	pDC->SelectStockObject(WHITE_PEN);
}

/////////////////////////////////////////////////////////////////////////////
// CTextToolView printing

BOOL CTextToolView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CTextToolView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CTextToolView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CTextToolView diagnostics

#ifdef _DEBUG
void CTextToolView::AssertValid() const
{
	CView::AssertValid();
}

void CTextToolView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTextToolDoc* CTextToolView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTextToolDoc)));
	return (CTextToolDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTextToolView message handlers

BOOL CTextToolView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	CBrush	brush(RGB(0,0,0));

	RECT	rect;
//	GetWindowRect(&rect);
	pDC->GetBoundsRect(&rect,0);
	pDC->FillRect(&rect,&brush);
	return true; //CView::OnEraseBkgnd(pDC);
}

void CTextToolView::OnFontSetfont() 
{
	LOGFONT			rLogFont;

	m_pFont->GetLogFont(&rLogFont);

	rLogFont.lfHeight = abs(rLogFont.lfHeight);

	CFontDialog		oDlog(&rLogFont);

	if (oDlog.DoModal() == IDOK)
	{
		oDlog.GetCurrentFont(&rLogFont);
		delete(m_pFont);
		m_pFont = new CFont;
		m_pFont->CreateFontIndirect(&rLogFont);

		//m_iFontPointSize = abs(rLogFont.lfHeight);

		int i,j;
		float k;

		i = rLogFont.lfHeight*72;
		j = iLogicalPixelsY;
		
		k = (float)i/(float)j;
		
		k -= 0.5f;

		m_iFontPointSize = -(int)k;

	}	
	InvalidateRect(NULL,true);
}

void CTextToolView::OnFontSavefontfiles() 
{
	LOGFONT			rLogFont;

	m_pFont->GetLogFont(&rLogFont);

	CString	szFaceNameT;
	CString szFaceName;
	szFaceNameT.Format("%s",rLogFont.lfFaceName);

	for (int i = 0; i < szFaceNameT.GetLength(); i++) {
	  if (szFaceNameT[i] != ' ')
	    szFaceName = szFaceName + szFaceNameT[i];
	}

	CString szType;

	switch(rLogFont.lfWeight)
	{
		case FW_DONTCARE: 
		case FW_NORMAL:
			break;

		case FW_THIN:
			szType = "Thin";
			break;
		case FW_EXTRALIGHT:
			szType = "ExtraLight";
			break;
		case FW_LIGHT: 
			szType = "Light";
			break;
		case FW_MEDIUM:
			szType = "Medium";
			break;
		case FW_SEMIBOLD: 
			szType = "SemiBold";
			break;
		case FW_BOLD:
			szType = "Bold";
			break;
		case FW_EXTRABOLD: 
			szType = "ExtraBold";
			break;
		case FW_BLACK:
			szType = "Black";
			break;

	}

	if (rLogFont.lfItalic != 0)
		szType += "Italic";

	if (rLogFont.lfUnderline != 0)
		szType += "Underline";

	if (rLogFont.lfStrikeOut != 0)
		szType += "Strike";

	CString	szSize;

	szSize.Format("_%d",m_iFontPointSize);

	CString	szFileName = szFaceName + szType + szSize;
 
	CFileDialog oFile(false,NULL,szFileName);

	if (oFile.DoModal() == IDOK)
	{

	  szFileName = oFile.GetFileName();

	  CString	szPNGName = oFile.GetPathName() + ".png";
	  CString	szMetricFileName = oFile.GetPathName() + ".fmt";

	  CDC	*pDC = GetDC();
	  CDC	*pDrawDC = pDC;//new CDC;

	  Invalidate(true);
	  // pDrawDC->CreateCompatibleDC(pDC);

	  if (!pDrawDC)
		  return;

	  int iImageY = m_iMaxY;

	  int iPictureY = iImageY;

	  int y2 = 4;

	  // find next greater power of two
	  while (iPictureY > y2) {
	    y2 <<= 1;
	  }
	  iPictureY = y2;

	  // fill the DC with black
	  CRect	rect;
	  CBrush	blackBrush(RGB(0,0,0));

  	  COLORREF      rPixel = RGB(0,0,0);

	  rect.SetRect(0,0,m_iMaxTextureWidth,iImageY);

	  pDrawDC->FillRect(&rect,&blackBrush);
	  OnDraw(pDrawDC);

	  std::ofstream* f = new std::ofstream(szPNGName, std::ios::out | std::ios::binary);

	  int temp = 0; //temporary values for binary file writing
	  char tempByte = 0;
	  int crc = 0;  //used for running CRC values

	  int w = 512, h = iPictureY;
	  unsigned long blength = (w + 1) * h * 4;
	  char* b = new char[blength];

	  // Write PNG headers
	  (*f) << "\211PNG\r\n\032\n";
#define	  PNGTAG(t_) ((((int)t_[0]) << 24) | \
		      (((int)t_[1]) << 16) | \
		      (((int)t_[2]) <<  8) | \
		        (int)t_[3])

	  // IHDR chunk
	  temp = htonl((int) 13);       //(length) IHDR is always 13 bytes long
	  f->write((char*) &temp, 4);
	  temp = htonl(PNGTAG("IHDR")); //(tag) IHDR
	  f->write((char*) &temp, 4);
	  crc = crc32(crc, (unsigned char*) &temp, 4);
	  temp = htonl(w);              //(data) Image width
	  f->write((char*) &temp, 4);
	  crc = crc32(crc, (unsigned char*) &temp, 4);
	  temp = htonl(h);	        //(data) Image height
	  f->write((char*) &temp, 4);
	  crc = crc32(crc, (unsigned char*) &temp, 4);
	  tempByte = 8;                 //(data) Image bitdepth (8 bits/sample = 24 bits/pixel)
	  f->write(&tempByte, 1);
	  crc = crc32(crc, (unsigned char*) &tempByte, 1);
	  tempByte = 6;                 //(data) Color type: RGBA = 6
	  f->write(&tempByte, 1);
	  crc = crc32(crc, (unsigned char*) &tempByte, 1);
	  tempByte = 0;
	  int i;
	  for (i = 0; i < 3; i++) { //(data) Last three tags are compression (only 0 allowed), filtering (only 0 allowed), and interlacing (we don't use it, so it's 0)
	    f->write(&tempByte, 1);
	    crc = crc32(crc, (unsigned char*) &tempByte, 1);
	  }
	  crc = htonl(crc);
	  f->write((char*) &crc, 4);    //(crc) write crc

	  // IDAT chunk

	  // fill buffer with black
	  for (i = 0; i < (long)blength; i++)
	    b[i] = 0;
	  // write image data over buffer
	  for (int y = 0; y <= m_iMaxY - 1; y++) {
	    const unsigned long line = y * (w * 4 + 1); //beginning of this line
	    b[line] = 0;  //filter type byte at the beginning of each scanline (0 = no filter, 1 = sub filter)
	    for (int x = 0; x < m_iMaxTextureWidth; x++) {
	      // Grab a reference to the current pixel
	      rPixel = pDrawDC->GetPixel(x, y);
	      // Pixel color values
	      b[line + x * 4 + 1] = GetRValue(rPixel);
	      b[line + x * 4 + 2] = GetGValue(rPixel);
	      b[line + x * 4 + 3] = GetBValue(rPixel);
	      // Write Alpha channel as average of RGB, since it's grayscale anyhow
	      b[line + x * 4 + 4] =(GetBValue(rPixel) + GetGValue(rPixel) + GetRValue(rPixel)) / 3;
	    }
	  }

	  unsigned long zlength = blength + 15;	    //length of bz[], will be changed by zlib to the length of the compressed string contained therein
	  unsigned char* bz = new unsigned char[zlength]; //just like b, but compressed; might get bigger, so give it room
	  // compress b into bz
	  compress2(bz, &zlength, reinterpret_cast<const unsigned char*>(b), blength, 5);
	  temp = htonl(zlength);                          //(length) IDAT length after compression
	  f->write((char*) &temp, 4);
	  temp = htonl(PNGTAG("IDAT"));                   //(tag) IDAT
	  f->write((char*) &temp, 4);
	  crc = crc32(crc = 0, (unsigned char*) &temp, 4);
	  f->write(reinterpret_cast<char*>(bz), zlength);  //(data) This line of pixels, compressed
	  crc = htonl(crc32(crc, bz, zlength));
	  f->write((char*) &crc, 4);                       //(crc) write crc

	  // tEXt chunk containing bzflag build/version
	  temp = htonl(28);//(length) tEXt is "Software\0BZFlag TextTool-W32"
	  f->write((char*) &temp, 4);
	  temp = htonl(PNGTAG("tEXt"));                   //(tag) tEXt
	  f->write((char*) &temp, 4);
	  crc = crc32(crc = 0, (unsigned char*) &temp, 4);
	  strcpy(b, "Software"); //(data) Keyword
	  f->write(reinterpret_cast<char*>(b), strlen(reinterpret_cast<const char*>(b)));
	  crc = crc32(crc, reinterpret_cast<const unsigned char*>(b), strlen(b));
	  tempByte = 0;					  //(data) Null character separator
	  f->write(&tempByte, 1);
	  crc = crc32(crc, (unsigned char*) &tempByte, 1);
	  strcpy((char*) b, "BZFlag TextTool-W32");       //(data) Text contents (build/version)
	  f->write(reinterpret_cast<char*>(b), strlen(reinterpret_cast<const char*>(b)));
	  crc = htonl(crc32(crc, reinterpret_cast<const unsigned char*>(b), strlen(b)));
	  f->write((char*) &crc, 4);                       //(crc) write crc

	  // IEND chunk
	  temp = htonl((int) 0);        //(length) IEND is always 0 bytes long
	  f->write((char*) &temp, 4);
	  temp = htonl(PNGTAG("IEND")); //(tag) IEND
	  f->write((char*) &temp, 4);
	  crc = htonl(crc32(crc = 0, (unsigned char*) &temp, 4));
	  //(data) IEND has no data field
	  f->write((char*) &crc, 4);     //(crc) write crc
	  delete [] bz;
	  delete [] b;
	  delete f;

	  // pDrawDC->DeleteDC();
	  // delete(pDrawDC);
	  ReleaseDC(pDC);

	  FILE *fp = fopen(szMetricFileName,"wt");
	  if (!fp)
		  return;

	  struct
	  {
		  int iInitalDist;
		  int iCharWidth;
		  int iWhiteSpaceDist;
		  int iStartX;
		  int iEndX;
		  int iStartY;
		  int iEndY;
	  }rFontMetrics;

	  int iNumChars = '}' - ' ';

		fprintf(fp,"NumChars: %d\nTextureWidth: %d\nTextureHeight: %d\nTextZStep: %d\n\n",iNumChars,m_iMaxTextureWidth,iPictureY,m_iTextureZStep);
	//  fwrite(&iNumChars,sizeof(int),1,fp);
	//  fwrite(&m_iMaxTextureWidth,sizeof(int),1,fp);
	//  fwrite(&iPictureY,sizeof(int),1,fp);
	//  fwrite(&m_iTextureZStep,sizeof(int),1,fp);

	  for (int iChar = 0; iChar <= iNumChars; iChar++)
	  {

		  rFontMetrics.iInitalDist = m_aWidths[iChar].abcA;
		  rFontMetrics.iCharWidth = m_aWidths[iChar].abcB;
		  rFontMetrics.iWhiteSpaceDist = m_aWidths[iChar].abcC;

		  rFontMetrics.iStartX = m_arGlyphExtents[iChar].iStartX;
		  rFontMetrics.iEndX = m_arGlyphExtents[iChar].iEndX;
		  rFontMetrics.iStartY = m_arGlyphExtents[iChar].iStartY;
		  rFontMetrics.iEndY = m_arGlyphExtents[iChar].iEndY;

			fprintf(fp,"Char: %c\nInitalDist: %d\nWidth: %d\nWhitespace: %d\n",iChar,rFontMetrics.iInitalDist,rFontMetrics.iCharWidth,rFontMetrics.iWhiteSpaceDist);
			fprintf(fp,"StartX: %d\nEndX: %d\nStartY: %d\nEndY: %d\n\n",rFontMetrics.iStartX,rFontMetrics.iEndX,rFontMetrics.iStartY,rFontMetrics.iEndY);

		  //fwrite(&rFontMetrics,sizeof(rFontMetrics),1,fp);
	  }
	  fclose(fp);
	}
	Invalidate(true);
}

void CTextToolView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	Invalidate(true);
	UpdateWindow();
}
