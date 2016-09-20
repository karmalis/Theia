
// GUIAppDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxeditbrowsectrl.h"


// CGUIAppDlg dialog
class CGUIAppDlg : public CDialogEx
{
// Construction
public:
	CGUIAppDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GUIAPP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	afx_msg LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);

	void ToggleHiddenFields();
	void CaptureInputs();

	CString _winTitle;
	bool _isConnected;

	// Database Username
	CString _dbUsername;
	CEdit _dbUsernameControl;
	// Database Password
	CString _dbPass;
	CEdit _dbPassControl;
	// Database Host
	CString _dbHost;
	CEdit _dbHostControl;
	// Database Port
	CString _dbPort;
	CEdit _dbPortControl;
public:
	// Button Controls
	afx_msg void OnBnClickedDbConnect();
	afx_msg void OnBnClickedButtonDownload();
	afx_msg void OnBnClickedButtonUpload();
	afx_msg void OnBnClickedButtonShowhidden();
protected:
	// File Path
	CString _filePath;
	CMFCEditBrowseCtrl _filepathControl;
	// Image ID to capture from the database
	CString _imageID;
	CEdit _imageIDControl;

	bool _hiddenFormFields;
	// Control Element For DBACTUAL
	CEdit _dbActualControl;
	CString _dbActual;
	// Control for DBTABLE
	CEdit _dbNameControl;
	CString _dbName;
	// Hidable Label Controls
	CStatic _groupHolderControl;
	CStatic _dbLabelControl;
	CStatic _dbTableControl;
};
