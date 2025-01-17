// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/fsdk_define.h"
#include "fpdfsdk/include/fsdk_mgr.h"
#include "fpdfsdk/include/fpdfxfa/fpdfxfa_util.h"
#include "fpdfsdk/include/fpdfxfa/fpdfxfa_doc.h"
#include "fpdfsdk/include/fpdfxfa/fpdfxfa_page.h"

CPDFXFA_Page::CPDFXFA_Page(CPDFXFA_Document* pDoc, int page_index)
    : m_pPDFPage(NULL),
      m_pXFAPageView(NULL),
      m_iPageIndex(page_index),
      m_pDocument(pDoc),
      m_iRef(1) {}

CPDFXFA_Page::~CPDFXFA_Page() {
  if (m_pPDFPage)
    delete m_pPDFPage;
  m_pPDFPage = NULL;
  m_pXFAPageView = NULL;
}

void CPDFXFA_Page::Release() {
  m_iRef--;
  if (m_iRef > 0)
    return;

  if (m_pDocument)
    m_pDocument->RemovePage(this);

  delete this;
}

FX_BOOL CPDFXFA_Page::LoadPDFPage() {
  if (!m_pDocument)
    return FALSE;
  CPDF_Document* pPDFDoc = m_pDocument->GetPDFDoc();
  if (pPDFDoc) {
    CPDF_Dictionary* pDict = pPDFDoc->GetPage(m_iPageIndex);
    if (pDict == NULL)
      return FALSE;
    if (m_pPDFPage) {
      if (m_pPDFPage->m_pFormDict == pDict)
        return TRUE;

      delete m_pPDFPage;
      m_pPDFPage = NULL;
    }

    m_pPDFPage = new CPDF_Page;
    m_pPDFPage->Load(pPDFDoc, pDict);
    m_pPDFPage->ParseContent();
    return TRUE;
  }

  return FALSE;
}

FX_BOOL CPDFXFA_Page::LoadXFAPageView() {
  if (!m_pDocument)
    return FALSE;
  IXFA_Doc* pXFADoc = m_pDocument->GetXFADoc();
  if (!pXFADoc)
    return FALSE;

  IXFA_DocView* pXFADocView = m_pDocument->GetXFADocView();
  if (!pXFADocView)
    return FALSE;

  IXFA_PageView* pPageView = pXFADocView->GetPageView(m_iPageIndex);
  if (!pPageView)
    return FALSE;

  if (m_pXFAPageView == pPageView)
    return TRUE;

  m_pXFAPageView = pPageView;
  (void)m_pXFAPageView->LoadPageView(nullptr);
  return TRUE;
}

FX_BOOL CPDFXFA_Page::LoadPage() {
  if (!m_pDocument || m_iPageIndex < 0)
    return FALSE;

  int iDocType = m_pDocument->GetDocType();
  switch (iDocType) {
    case DOCTYPE_PDF:
    case DOCTYPE_STATIC_XFA: {
      return LoadPDFPage();
    }
    case DOCTYPE_DYNAMIC_XFA: {
      return LoadXFAPageView();
    }
    default:
      return FALSE;
  }

  return FALSE;
}

FX_BOOL CPDFXFA_Page::LoadPDFPage(CPDF_Dictionary* pageDict) {
  if (!m_pDocument || m_iPageIndex < 0 || !pageDict)
    return FALSE;

  if (m_pPDFPage)
    delete m_pPDFPage;

  m_pPDFPage = new CPDF_Page();
  m_pPDFPage->Load(m_pDocument->GetPDFDoc(), pageDict);
  m_pPDFPage->ParseContent();

  return TRUE;
}

FX_FLOAT CPDFXFA_Page::GetPageWidth() {
  ASSERT(m_pDocument != NULL);

  if (!m_pPDFPage && !m_pXFAPageView)
    return 0.0f;

  int nDocType = m_pDocument->GetDocType();
  switch (nDocType) {
    case DOCTYPE_DYNAMIC_XFA: {
      if (m_pXFAPageView) {
        CFX_RectF rect;
        m_pXFAPageView->GetPageViewRect(rect);
        return rect.width;
      }
    } break;
    case DOCTYPE_STATIC_XFA:
    case DOCTYPE_PDF: {
      if (m_pPDFPage)
        return m_pPDFPage->GetPageWidth();
    } break;
    default:
      return 0.0f;
  }

  return 0.0f;
}

FX_FLOAT CPDFXFA_Page::GetPageHeight() {
  ASSERT(m_pDocument != NULL);

  if (!m_pPDFPage && !m_pXFAPageView)
    return 0.0f;

  int nDocType = m_pDocument->GetDocType();
  switch (nDocType) {
    case DOCTYPE_PDF:
    case DOCTYPE_STATIC_XFA: {
      if (m_pPDFPage)
        return m_pPDFPage->GetPageHeight();
    } break;
    case DOCTYPE_DYNAMIC_XFA: {
      if (m_pXFAPageView) {
        CFX_RectF rect;
        m_pXFAPageView->GetPageViewRect(rect);
        return rect.height;
      }
    } break;
    default:
      return 0.0f;
  }

  return 0.0f;
}

void CPDFXFA_Page::DeviceToPage(int start_x,
                                int start_y,
                                int size_x,
                                int size_y,
                                int rotate,
                                int device_x,
                                int device_y,
                                double* page_x,
                                double* page_y) {
  ASSERT(m_pDocument != NULL);

  if (!m_pPDFPage && !m_pXFAPageView)
    return;

  CFX_Matrix page2device;
  CFX_Matrix device2page;
  FX_FLOAT page_x_f, page_y_f;

  GetDisplayMatrix(page2device, start_x, start_y, size_x, size_y, rotate);

  device2page.SetReverse(page2device);
  device2page.Transform((FX_FLOAT)(device_x), (FX_FLOAT)(device_y), page_x_f,
                        page_y_f);

  *page_x = (page_x_f);
  *page_y = (page_y_f);
}

void CPDFXFA_Page::PageToDevice(int start_x,
                                int start_y,
                                int size_x,
                                int size_y,
                                int rotate,
                                double page_x,
                                double page_y,
                                int* device_x,
                                int* device_y) {
  if (!m_pPDFPage && !m_pXFAPageView)
    return;

  CFX_Matrix page2device;
  FX_FLOAT device_x_f, device_y_f;

  GetDisplayMatrix(page2device, start_x, start_y, size_x, size_y, rotate);

  page2device.Transform(((FX_FLOAT)page_x), ((FX_FLOAT)page_y), device_x_f,
                        device_y_f);

  *device_x = FXSYS_round(device_x_f);
  *device_y = FXSYS_round(device_y_f);
}

void CPDFXFA_Page::GetDisplayMatrix(CFX_Matrix& matrix,
                                    int xPos,
                                    int yPos,
                                    int xSize,
                                    int ySize,
                                    int iRotate) const {
  ASSERT(m_pDocument != NULL);

  if (!m_pPDFPage && !m_pXFAPageView)
    return;

  int nDocType = m_pDocument->GetDocType();
  switch (nDocType) {
    case DOCTYPE_DYNAMIC_XFA: {
      if (m_pXFAPageView) {
        CFX_Rect rect;
        rect.Set(xPos, yPos, xSize, ySize);
        m_pXFAPageView->GetDisplayMatrix(matrix, rect, iRotate);
      }
    } break;
    case DOCTYPE_PDF:
    case DOCTYPE_STATIC_XFA: {
      if (m_pPDFPage) {
        m_pPDFPage->GetDisplayMatrix(matrix, xPos, yPos, xSize, ySize, iRotate);
      }
    } break;
    default:
      return;
  }
}
