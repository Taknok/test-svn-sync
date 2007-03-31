/****************************************************************************

	CEditFoilDlg class
	Copyright (C) 2004 Andr� Deperrois xflr5@yahoo.com

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************/

#pragma once

// EditFoilDlg.h : header file
//
#include "../misc/EditListCtrl.h"
#include "Foil.h"

/////////////////////////////////////////////////////////////////////////////
// CEditFoilDlg dialog

class CEditFoilDlg : public CDialog
{
	friend class CXDirect;
	friend class CAFoil;

// Construction
public:
	void SetList();
	void SetSelection(int sel);
	CEditFoilDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditFoilDlg)
	enum { IDD = IDD_EDITFOILDLG };
	CButton	m_ctrlOK;
	CEditListCtrl	m_ctrlCoordList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditFoilDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool m_bApplied, m_bModified;

	CFoil* m_pMemFoil;
	CFoil* m_pBufferFoil;

	CWnd *m_pChildView;

	void ReadSectionData(int sel, double &X, double &Y);
	// Generated message map functions
	//{{AFX_MSG(CEditFoilDlg)
	afx_msg void OnDeletePt();
	virtual BOOL OnInitDialog();
	afx_msg void OnNMClickCoordList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickCoordList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnEndLabelEditCoordList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRestore();
	afx_msg void OnApply();
	afx_msg void OnAppend();
	afx_msg void OnInsertPt();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

