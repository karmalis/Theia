
// GUIAppDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GUIApp.h"
#include "GUIAppDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGUIAppDlg dialog



CGUIAppDlg::CGUIAppDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_GUIAPP_DIALOG, pParent)
	, _dbUsername(_T(""))
	, _dbPass(_T(""))
	, _dbHost(_T(""))
	, _dbPort(_T(""))
	, _filePath(_T(""))
	, _imageID(_T(""))
	, _hiddenFormFields(true)
	, _isConnected(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGUIAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_DBUSERNAME, _dbUsername);
	DDX_Text(pDX, IDC_EDIT_DBPASS, _dbPass);
	DDX_Text(pDX, IDC_EDIT_DBHOST, _dbHost);
	DDX_Text(pDX, IDC_EDIT_DBPORT, _dbPort);
	DDX_Text(pDX, IDC_FILEPATH, _filePath);
	DDX_Text(pDX, IDC_EDIT_IMAGEID, _imageID);

	DDX_Control(pDX, IDC_EDIT_DBUSERNAME, _dbUsernameControl);
	DDX_Control(pDX, IDC_EDIT_DBPASS, _dbPassControl);
	DDX_Control(pDX, IDC_EDIT_DBHOST, _dbHostControl);
	DDX_Control(pDX, IDC_EDIT_DBPORT, _dbPortControl);
	DDX_Control(pDX, IDC_EDIT_IMAGEID, _imageIDControl);

	DDX_Control(pDX, IDC_EDIT_DBACTUAL, _dbActualControl);
	DDX_Control(pDX, IDC_EDIT_DBTABLE, _dbNameControl);
	DDX_Control(pDX, IDC_GROUP_HIDDEN_LABEL, _groupHolderControl);
	DDX_Control(pDX, IDC_DBACTUAL_LABEL, _dbLabelControl);
	DDX_Control(pDX, IDC_TABLE_LABEL, _dbTableControl);
	DDX_Control(pDX, IDC_FILEPATH, _filepathControl);
}

BEGIN_MESSAGE_MAP(CGUIAppDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_DB_CONNECT, &CGUIAppDlg::OnBnClickedDbConnect)
	ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD, &CGUIAppDlg::OnBnClickedButtonDownload)
	ON_BN_CLICKED(IDC_BUTTON_UPLOAD, &CGUIAppDlg::OnBnClickedButtonUpload)
	ON_BN_CLICKED(IDC_BUTTON_SHOWHIDDEN, &CGUIAppDlg::OnBnClickedButtonShowhidden)
END_MESSAGE_MAP()


// CGUIAppDlg message handlers

BOOL CGUIAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, FALSE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	GetWindowText(_winTitle);
	CString newTitle = _winTitle + CString(" [Not Connected]");
	SetWindowText(newTitle);

	_filepathControl.EnableFileBrowseButton(_T("IMG"), _T("|*.jpg|*.png|*.gif|*.bmp||"));

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGUIAppDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGUIAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CGUIAppDlg::ToggleHiddenFields()
{
	//this->_dbActualControl.ShowWindow
	if (this->_hiddenFormFields)
	{
		this->_dbActualControl.ShowWindow(true);
		this->_dbNameControl.ShowWindow(true);
		this->_groupHolderControl.ShowWindow(true);
		this->_dbLabelControl.ShowWindow(true);
		this->_dbTableControl.ShowWindow(true);
	} else {
		this->_dbActualControl.ShowWindow(false);
		this->_dbNameControl.ShowWindow(false);
		this->_groupHolderControl.ShowWindow(false);
		this->_dbLabelControl.ShowWindow(false);
		this->_dbTableControl.ShowWindow(false);
	}

	this->_hiddenFormFields = !this->_hiddenFormFields;
}

void CGUIAppDlg::CaptureInputs()
{
	_dbUsernameControl.GetWindowTextW(_dbUsername);
	_dbPassControl.GetWindowTextW(_dbPass);
	_dbHostControl.GetWindowTextW(_dbHost);
	
	_dbPortControl.GetWindowTextW(_dbPort);
	_imageIDControl.GetWindowTextW(_imageID);
	_filepathControl.GetWindowTextW(_filePath);

	_dbActualControl.GetWindowTextW(_dbActual);
	_dbNameControl.GetWindowTextW(_dbName);
}

typedef bool(*Connect)(
	const char* username,
	const char* password,
	const char* hostname,
	unsigned short port
	);

typedef int(*Upload)(
	const unsigned char* imageData,
	unsigned int imageDataSize,
	const char* imageName
	);

typedef bool(*Download)(
	int id,
	unsigned char** data,
	unsigned int* dataSize,
	char* filename
	);

typedef void(*FreeImageData)(
	const unsigned char* data
	);

typedef const char*(*FetchLastError)();

typedef void(*SetTableAndDB)(const char* database, const char* table);

void CGUIAppDlg::OnBnClickedDbConnect()
{
	CString initialTitle = _winTitle + CString(" [Connecting...]");
	SetWindowText(initialTitle);

	this->CaptureInputs();

	if (this->_dbUsername.Trim().GetLength() == 0)
	{
		MessageBox(_T("Please enter a username")); return;
	}

	if (this->_dbPass.Trim().GetLength() == 0)
	{
		MessageBox(_T("Please enter a password")); return;
	}

	if (this->_dbHost.Trim().GetLength() == 0)
	{
		MessageBox(_T("Please enter a host")); return;
	}

	if (this->_dbPort.Trim().GetLength() == 0)
	{
		MessageBox(_T("Please enter a port")); return;
	}

	Connect connectFunc = (Connect)(GetProcAddress(theApp.connectorMod, "Connect"));
	FetchLastError lastErrFunc = (FetchLastError)(GetProcAddress(theApp.connectorMod, "FetchLastError"));
	SetTableAndDB setTableAndDbFunc = (SetTableAndDB)(GetProcAddress(theApp.connectorMod, "SetTableAndDB"));


	if (this->_dbActual.Trim().GetLength() != 0 || this->_dbName.Trim().GetLength() != 0)
	{
		setTableAndDbFunc(
			CT2A(this->_dbActual.Trim()),
			CT2A(this->_dbName.Trim())
			);
	}

	if (!connectFunc(
		CT2A(this->_dbUsername.Trim()),
		CT2A(this->_dbPass.Trim()),
		CT2A(this->_dbHost.Trim()),
		_ttoi(this->_dbPort.Trim())
		)) {
		const char* lastError = lastErrFunc();
		MessageBox(_T("Could not connect to server: ") + CString(lastError));

		CString newTitle = _winTitle + CString(" [Not Connected]");
		SetWindowText(newTitle);
		_isConnected = false;

	}
	else {
		MessageBox(_T("Connection successful!"));

		CString newTitle = _winTitle + CString(" [Connected]");
		SetWindowText(newTitle);
		_isConnected = true;
	}
}

#include <fstream>

void CGUIAppDlg::OnBnClickedButtonDownload()
{
	this->CaptureInputs();
	if (!_isConnected)
	{
		this->OnBnClickedDbConnect();
		if (!_isConnected) return;
	}

	if (_imageID.Trim().GetLength() == 0)
	{
		MessageBox(_T("Please specify and image ID"));
		return;
	}

	Download downloadFunc = (Download)(GetProcAddress(theApp.connectorMod, "Download"));
	FreeImageData freeImageFunc = (FreeImageData)(GetProcAddress(theApp.connectorMod, "FreeImageData"));
	FetchLastError lastErrFunc = (FetchLastError)(GetProcAddress(theApp.connectorMod, "FetchLastError"));

	unsigned char* imageData = NULL;
	unsigned int imageSize = 0;
	char* filename = new char[128];

	bool downloadResult = downloadFunc(
		_ttoi(_imageID),
		&imageData,
		&imageSize,
		filename
		);

	if (!downloadResult)
	{
		const char* lastError = lastErrFunc();
		MessageBox(_T("Could not download image: ") + CString(lastError));
		return;

	}

	CString cFName(filename);
	CString ext = cFName.Mid(cFName.ReverseFind('.') + 1);

	char strFilter[] = {"|*.jpg|*.png|*.gif|*.bmp||"};
	CFileDialog FileDlg(FALSE, ext, cFName, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, CString("|*.jpg|*.png|*.gif|*.bmp||"));
	FileDlg.m_ofn.lpstrTitle = _T("Save Image As");

	if (FileDlg.DoModal() == IDOK)
	{
		CString target = FileDlg.GetPathName();
		std::basic_ofstream<unsigned char> output(CT2A(target), std::ofstream::binary);
		output.write(imageData, imageSize);
		output.close();
		MessageBox(_T("Image has been saved"));
	}

	freeImageFunc(imageData);
	delete filename;
}


void CGUIAppDlg::OnBnClickedButtonUpload()
{
	this->CaptureInputs();
	if (!_isConnected)
	{
		this->OnBnClickedDbConnect();
		if (!_isConnected) return;
	}

	if (_filePath.Trim().GetLength() == 0)
	{
		MessageBox(_T("Please specify an image path"));
		return;
	}

	std::basic_ifstream<unsigned char> image(_filePath, std::ios::binary | std::ios::ate);
	if (!image.is_open())
	{
		MessageBox(_T("Invalid or corrupted file at filepath")); 
		return;
	}

	CString filename = _filePath.Mid(_filePath.ReverseFind('\\') + 1);

	unsigned int size;
	unsigned char * fileBuff;
	size = image.tellg();
	image.seekg(0, image.beg);
	fileBuff = new  unsigned char[size];
	image.read(fileBuff, size);
	image.close();

	// Check the file size if not larger than BLOB
	if (size > 65535)
	{
		MessageBox(_T("Image is larger than 65535 bytes"));
		return;
	}

	if (filename.GetLength() > 128)
	{
		MessageBox(_T("Filename of the image is larger than allowed 128 characters"));
		return;
	}

	Upload uploadFunc = (Upload)(GetProcAddress(theApp.connectorMod, "Upload"));
	int id = uploadFunc(
		fileBuff,
		size,
		CT2A(filename)
		);
	delete fileBuff;

	if ( id == -1) {
		FetchLastError lastErrFunc = (FetchLastError)(GetProcAddress(theApp.connectorMod, "FetchLastError"));
		const char* lastError = lastErrFunc();
		MessageBox(_T("Failed to upload image: ") + CString(lastError));

		return;
	}
	else {
		CString idString;
		idString.Format(_T("%d"), id);
		MessageBox(_T("Image uploaded successfuly! Resulting ID: ") + idString);
	}
}


void CGUIAppDlg::OnBnClickedButtonShowhidden()
{
	this->ToggleHiddenFields();
}
