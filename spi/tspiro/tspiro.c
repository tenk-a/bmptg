/**
 *	@file	tspiro.cpp
 *	@brief	tspiro.exe専用のsusieプラグイン用に、レジストリを設定するツール.
 *	@author	Masashi KITAMURA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <commctrl.h>
#include <mbctype.h>
#include "resource.h"
#include "../spi/regopt.h"
#include "gr.h"

#define WIN_W		(14*16+ 8)
#define WIN_H		(14*16+ 8)

const char	*app_name = "TSPIRO";
static volatile HINSTANCE	main_hInst;
static volatile HWND		main_hClutWnd;
static volatile HWND		main_hDlg;
static volatile HWND		main_hStatWnd;
static volatile int 		main_regrslt_flag;
static volatile int 		main_subwin_seq;
static volatile int			subwin_update = 1;

static volatile regopt_t 	regopt;

typedef struct RegRsltDat {
	regopt_s_t	regrslt;
	char 		regfname[MAX_PATH*2+4];
	DWORD		regclut[REGOPT_CLUT_NUM];
} RegRsltDat;

static volatile RegRsltDat	regrsltDat;
static volatile RegRsltDat	regrsltDat_sv;

static CHOOSECOLOR 			main_chooseColor = {0};



//#define	GR_RGB(r,g,b)			((((unsigned char)(0))<<24)+(((unsigned char)(r))<<16)+(((unsigned char)(g))<<8)+((unsigned char)(b)))

#define BBBB(a,b,c,d)	((((BYTE)(a))<<24)+(((BYTE)(b))<<16)+(((BYTE)(c))<<8)+((BYTE)(d)))


static LRESULT CALLBACK winMainProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam);
static BOOL CALLBACK panel_proc(HWND hdwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void subwin();
static int clamp(int n, int min, int max);
static int RegOpt2panel(HWND hdwnd, regopt_t *ro);
static int getDlgChkBox(HWND hdwnd, int idc);
static int setDlgChkBox(HWND hdwnd, int idc, int n);
static int dlgText2xval(HWND hdwnd, int idc);
static int dlgText2val(HWND hdwnd, int idc);
static int xval2dlgText(HWND hdwnd, int idc, int val);
static int val2dlgText(HWND hdwnd, int idc, int val);
static void chooseColor_init(HWND hWnd, COLORREF color);
static void setChooseColor(HWND hWnd, DWORD* pCol, int item);
static int	RegOptPut(regopt_t *ro);
static int	RegOptGet(regopt_t *ro);
static void SetDfltOpt(regopt_t *ro);
static int  RegRsltGet(regopt_s_t *rs, DWORD *clut, char *fname);
static void DrawClutData(regopt_s_t *rs, DWORD clut[]);
static void PaintClutStatus(regopt_s_t *rs, DWORD clut[]);
static char *fname_baseName(char *adr);
static int  isJapanese(void);



/** 初っ端に起動される
 */
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	MSG 		msg = {0};
	HANDLE		hMutex;
	//WNDCLASS 	wndclass;
	WNDCLASSEX 	wndclass;

	nCmdShow;
	lpCmdLine;
	hPrevInst;

	/* 多重起動防止. */
	hMutex = CreateMutex(NULL, 1, app_name);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return 0;

	/* ウィンドウクラスの登録 */
	wndclass.cbSize 		= sizeof(WNDCLASSEX);
	main_hInst 				= hInst;
	wndclass.lpszClassName	= app_name;
	wndclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
//	wndclass.hIcon			= LoadIcon(main_hInst, 0/*MAKEINTRESOURCE(IDI_MAIN_ICON)*/);
	wndclass.hIcon			= LoadIcon(main_hInst, MAKEINTRESOURCE(IDI_MAIN_ICON));
	wndclass.hIconSm		= LoadIcon(main_hInst, MAKEINTRESOURCE(IDI_MAIN_ICON));
	wndclass.lpszMenuName	= MAKEINTRESOURCE(0/*IDR_MENU*/);
	wndclass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hInstance		= hInst;
	wndclass.style			= CS_BYTEALIGNCLIENT|CS_VREDRAW|CS_HREDRAW; //CS_VREDRAW | CS_HREDRAW;
	wndclass.lpfnWndProc	= (WNDPROC)winMainProc;
	wndclass.cbClsExtra 	= 0;
	wndclass.cbWndExtra 	= 0;
	//if (RegisterClassEx(&wndclass) == 0)
	if (RegisterClassEx(&wndclass) == 0)
		return 0;

	RegOptGet(&regopt);
	regopt.enable |= 3;

	main_hDlg = CreateDialog(main_hInst, MAKEINTRESOURCE(IDD_TSPIRO), main_hClutWnd, (DLGPROC)panel_proc);
	//DialogBox(main_hInst, MAKEINTRESOURCE(IDD_TSPIRO), main_hClutWnd, (DLGPROC)panel_proc);
	chooseColor_init(main_hDlg, 0);

  #if 1
	// メッセージループ
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (IsDialogMessage(main_hDlg, &msg) == 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		subwin();
		Sleep(16);
	}
  #endif
	/* 終了処理 */

	CloseHandle(hMutex);
	return 0/*msg.wParam*/;
}



static void subwin()
{
	if (main_subwin_seq) {				// Clut窓の擬似開閉
		if (main_subwin_seq == 2 && main_hClutWnd == NULL) {	// Clut窓を空ける
			RECT r;
			int  x, y, w, h;
			GetWindowRect(main_hDlg, &r);
			x = r.left;
			y = r.bottom + 4;
			r.left   = 0;
			r.top    = 0;
			r.right  = WIN_W;
			r.bottom = WIN_H;
			//x w = (WIN_W + GetSystemMetrics(SM_CXSIZEFRAME)*2);
			//x h = (WIN_H + GetSystemMetrics(SM_CYSIZEFRAME)*2 + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYCAPTION));
			AdjustWindowRectEx(&r, WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX, 0, 0);
			w = r.right  - r.left;
			h = r.bottom - r.top;
			main_hClutWnd = CreateWindowEx(0, app_name, "TSPIRO clut", WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX, x, y, w, h, NULL, NULL, main_hInst, NULL);
			//main_hStatWnd = CreateStatusWindow(WS_CHILD|WS_VISIBLE, NULL, main_hClutWnd, 0);
			main_hStatWnd = CreateWindow(STATUSCLASSNAME, "", WS_CHILD|WS_VISIBLE, 0,0,0,0, main_hClutWnd, NULL, main_hInst, NULL);
			GetWindowRect(main_hStatWnd, &r);
			CloseWindow(main_hStatWnd);
			h += r.bottom - r.top;
			SetWindowPos(main_hClutWnd, main_hDlg, x,y,w,h, 0);
			ShowWindow(main_hClutWnd, SW_SHOWNORMAL);
			main_hStatWnd = CreateWindow(STATUSCLASSNAME, "", WS_CHILD|WS_VISIBLE, 0,0,0,0, main_hClutWnd, NULL, main_hInst, NULL);
			gr_init(main_hClutWnd, WIN_W,WIN_H, 0);
			UpdateWindow(main_hClutWnd);
			gr_update();
			subwin_update = 1;
		} else if (main_subwin_seq == 1) {		// Clut窓を閉じる
			gr_term();
			DestroyWindow(main_hClutWnd);
			memset(&regrsltDat_sv, 0, sizeof regrsltDat_sv);
			main_hClutWnd = NULL;
		}
		main_subwin_seq = 0;
	}
	if (main_regrslt_flag && main_hClutWnd) {		// 特殊SPIからのデータを読み取る
		if (RegRsltGet(&regrsltDat.regrslt, regrsltDat.regclut, regrsltDat.regfname)) {	// 情報が更新された。
			if (memcmp(&regrsltDat, &regrsltDat_sv, sizeof regrsltDat) != 0) {
				InvalidateRect(main_hClutWnd, NULL, 0);
				//x regrsltDat_sv = regrsltDat;
				memcpy(&regrsltDat_sv, &regrsltDat, sizeof regrsltDat_sv);
				subwin_update = 1;
			}
		}
	}
}



/** ウィンドウ・メッセージ処理
 */
static LRESULT CALLBACK winMainProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam)
{
	switch (msg) {
	case WM_CREATE:
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		//PostQuitMessage(0);
		main_hClutWnd 		= NULL;
		main_regrslt_flag 	= 0;
		main_subwin_seq 	= 0;
		break;

	case WM_PAINT:
		if (subwin_update) {
			DrawClutData(&regrsltDat.regrslt, regrsltDat.regclut);
//			PaintClutStatus(&regrsltDat.regrslt, regrsltDat.regclut);
			subwin_update = 0;
		}
		gr_update();
		PaintClutStatus(&regrsltDat.regrslt, regrsltDat.regclut);
		break;

	case WM_MOVE:
		// Retrieve the window position after a move
		gr_wm_move(hWnd);
//		PaintClutStatus(&regrsltDat.regrslt, regrsltDat.regclut);
		break;

	case WM_SIZE:
		if (wParam == SIZE_MAXHIDE || wParam == SIZE_MINIMIZED) {
		} else {
			gr_wm_move(hWnd);
//			PaintClutStatus(&regrsltDat.regrslt, regrsltDat.regclut);
			//gr_winRectChk(hWnd);
		}
		break;

	//case WM_GETMINMAXINFO:
		//gr_wm_move(hWnd);
		//break;

	case WM_COMMAND:
		//switch (LOWORD(wParam)) {
		//case IDM_EXIT:
		//	PostQuitMessage(0);
		//	break;
		//}
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}



static __inline int clamp(int n, int min, int max)
{
	if (n < min)
		n = min;
	else if (max < n)
		n = max;
	return n;
}



/** パネルの処理
 */
static BOOL CALLBACK panel_proc(HWND hdwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	regopt_t 	*ro = &regopt;
	int			c   = 0;
	lParam;

	switch (msg) {
	case WM_INITDIALOG:
		RegOpt2panel(hdwnd, ro);
		break;

	case WM_CLOSE:
		if ((regopt.enable & 4) == 0)
			regopt.enable = 0;
		RegOptPut(ro);
		PostQuitMessage(0);
		//EndDialog(hdwnd, 0);
		return 1;

	case WM_PAINT:
		//ShowWindow(hdwnd, SW_SHOWNOACTIVATE);
		//return 1;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_DFLT_LOAD:
			SetDfltOpt(ro);
			RegOpt2panel(hdwnd, ro);
			break;
		case IDC_RO_ENABLE:
			ro->enable &= ~3;
			ro->enable |= 3 * (getDlgChkBox(hdwnd, IDC_RO_ENABLE) == 0);
			break;
		case IDC_CONT_PARAM:
			ro->enable &= ~4;
			ro->enable |= getDlgChkBox(hdwnd, IDC_CONT_PARAM) << 2;
			break;
		case IDC_RO_DIB32:
			c = getDlgChkBox(hdwnd, IDC_RO_DIB32);
			ro->bpp32use &=  ~1;
			ro->bpp32use |=  c;
			RegOpt2panel(hdwnd, ro);
			break;
		case IDC_RO_DIB32_BPP8:
			c = getDlgChkBox(hdwnd, IDC_RO_DIB32_BPP8);
			ro->bpp32use &=  ~2;
			ro->bpp32use |=  c << 1;
			RegOpt2panel(hdwnd, ro);
			break;
		case IDC_RO_ALP_MODE:
			ro->alpMode = getDlgChkBox(hdwnd, IDC_RO_ALP_MODE);
			break;
		case IDC_RO_ALP_REV:
			ro->alpRev = getDlgChkBox(hdwnd, IDC_RO_ALP_REV);
			break;
		case IDC_RO_ALP_HALF:
			ro->alpHalf = getDlgChkBox(hdwnd, IDC_RO_ALP_HALF);
			break;
		case IDC_RO_RNGCHK:
			ro->rngChk = getDlgChkBox(hdwnd, IDC_RO_RNGCHK);
			break;
		case IDC_RO_BG_GRID:
			ro->bgGridMode = getDlgChkBox(hdwnd, IDC_RO_BG_GRID);
			break;
		case IDC_RO_GRID_FLG:
			ro->gridFlg = getDlgChkBox(hdwnd, IDC_RO_GRID_FLG);
			break;
		case IDC_RO_GRID_W:
			ro->gridW = dlgText2val(hdwnd, IDC_RO_GRID_W);
			ro->gridW = clamp(ro->gridW, 0, 1024);
			break;
		case IDC_RO_GRID_H:
			ro->gridH = dlgText2val(hdwnd, IDC_RO_GRID_H);
			ro->gridH = clamp(ro->gridH, 0, 1024);
			break;
		case IDC_RO_TONE:
			ro->tone = dlgText2val(hdwnd, IDC_RO_TONE);
			ro->tone = clamp(ro->tone, 0, 0x7FFF);
			break;
		case IDC_RO_CLUT_OFS:
			ro->clutOfs = dlgText2val(hdwnd, IDC_RO_CLUT_OFS);
			break;
		case IDC_RO_CLUT_KEY:
			ro->clutKey = dlgText2val(hdwnd, IDC_RO_CLUT_KEY);
			ro->clutKey = clamp(ro->clutKey, 0, 255);
			break;

		case IDC_RO_COL_KEY:
			ro->colKey = dlgText2xval(hdwnd, IDC_RO_COL_KEY);
			break;
		case IDC_RO_BG_COL1:
			ro->bgCol1 = dlgText2xval(hdwnd, IDC_RO_BG_COL1);
			break;
		case IDC_RO_BG_COL2:
			ro->bgCol2 = dlgText2xval(hdwnd, IDC_RO_BG_COL2);
			break;
		case IDC_RO_GRID_COL:
			ro->gridCol = dlgText2xval(hdwnd, IDC_RO_GRID_COL);
			break;

		case IDC_RO_BG_COL_BTN1:
			setChooseColor(hdwnd, &ro->bgCol1, IDC_RO_BG_COL1);
			break;
		case IDC_RO_BG_COL_BTN2:
			setChooseColor(hdwnd, &ro->bgCol2, IDC_RO_BG_COL2);
			break;
		case IDC_RO_GRID_COL_BTN:
			setChooseColor(hdwnd, &ro->gridCol, IDC_RO_GRID_COL);
			break;

		case IDC_RO_KEY_MODE_0:
			if (getDlgChkBox(hdwnd, IDC_RO_KEY_MODE_0))
				ro->keyMode = 0;
			RegOpt2panel(hdwnd, ro);
			break;
		case IDC_RO_KEY_MODE_1:
			if (getDlgChkBox(hdwnd, IDC_RO_KEY_MODE_1))
				ro->keyMode = 1;
			RegOpt2panel(hdwnd, ro);
			break;
		case IDC_RO_KEY_MODE_2:
			if (getDlgChkBox(hdwnd, IDC_RO_KEY_MODE_2))
				ro->keyMode = 2;
			RegOpt2panel(hdwnd, ro);
			break;
		case IDC_RO_ALP_TYPE_0:
			if (getDlgChkBox(hdwnd, IDC_RO_ALP_TYPE_0))
				ro->alpType = 0;
			RegOpt2panel(hdwnd, ro);
			break;
		case IDC_RO_ALP_TYPE_1:
			if (getDlgChkBox(hdwnd, IDC_RO_ALP_TYPE_1))
				ro->alpType = 1;
			RegOpt2panel(hdwnd, ro);
			break;
		case IDC_RO_ALP_TYPE_2:
			if (getDlgChkBox(hdwnd, IDC_RO_ALP_TYPE_2))
				ro->alpType = 2;
			RegOpt2panel(hdwnd, ro);
			break;
		case IDC_RO_ALP_TYPE_3:
			if (getDlgChkBox(hdwnd, IDC_RO_ALP_TYPE_3))
				ro->alpType = 3;
			RegOpt2panel(hdwnd, ro);
			break;

		case IDC_CLUT_DISP:
			if (main_subwin_seq == 0) {
				main_regrslt_flag ^= 1;
				main_subwin_seq   = 1 + main_regrslt_flag;
			}
			break;

		case IDC_MAPOFFSW:
			ro->mapOffSw = getDlgChkBox(hdwnd, IDC_MAPOFFSW);
			RegOpt2panel(hdwnd, ro);
			break;

		case IDC_USAGE:
			ShellExecute(NULL, "open", "tspiro.htm", NULL, NULL, SW_SHOWNORMAL);
			break;

		default:
			return 0;
		}
		RegOptPut(ro);
		return 1;
	default:
		;
	}
	return 0;
}




/*-----------------------------------------------------------------------*/

/** RegOptの情報をパネル(画面)に反映.
 */
static int RegOpt2panel(HWND hdwnd, regopt_t *ro)
{
	setDlgChkBox(hdwnd, IDC_RO_ENABLE, (ro->enable&3) == 0);
	setDlgChkBox(hdwnd, IDC_CONT_PARAM, (ro->enable&4) != 0);
	setDlgChkBox(hdwnd, IDC_RO_DIB32,  ro->bpp32use & 1);
	setDlgChkBox(hdwnd, IDC_RO_DIB32_BPP8, (ro->bpp32use >> 1) & 1);
	setDlgChkBox(hdwnd, IDC_RO_ALP_MODE, ro->alpMode);
	setDlgChkBox(hdwnd, IDC_RO_ALP_REV, ro->alpRev);
	setDlgChkBox(hdwnd, IDC_RO_ALP_HALF, ro->alpHalf);
	setDlgChkBox(hdwnd, IDC_RO_RNGCHK,	ro->rngChk);
	setDlgChkBox(hdwnd, IDC_RO_BG_GRID, ro->bgGridMode);
	setDlgChkBox(hdwnd, IDC_RO_GRID_FLG, ro->gridFlg);
	setDlgChkBox(hdwnd, IDC_MAPOFFSW, ro->mapOffSw);

	val2dlgText(hdwnd, IDC_RO_GRID_W, ro->gridW);
	val2dlgText(hdwnd, IDC_RO_GRID_H, ro->gridH);
	val2dlgText(hdwnd, IDC_RO_TONE, ro->tone);
	val2dlgText(hdwnd, IDC_RO_ALP_VAL, ro->alpVal);
	val2dlgText(hdwnd, IDC_RO_CLUT_OFS, ro->clutOfs);
	val2dlgText(hdwnd, IDC_RO_CLUT_KEY, ro->clutKey);

	xval2dlgText(hdwnd, IDC_RO_COL_KEY, ro->colKey);
	xval2dlgText(hdwnd, IDC_RO_BG_COL1, ro->bgCol1);
	xval2dlgText(hdwnd, IDC_RO_BG_COL2, ro->bgCol2);
	xval2dlgText(hdwnd, IDC_RO_GRID_COL, ro->gridCol);

	//val2dlgText(hdwnd, IDC_RO_KEY_MODE2, ro->alpMode);
	//val2dlgText(hdwnd, IDC_RO_ALP_TYPE2, ro->alpType);

	// key mode
	setDlgChkBox(hdwnd, IDC_RO_KEY_MODE_0, ro->keyMode == 0);
	setDlgChkBox(hdwnd, IDC_RO_KEY_MODE_1, ro->keyMode == 1);
	setDlgChkBox(hdwnd, IDC_RO_KEY_MODE_2, ro->keyMode == 2);

	// alp typ
	setDlgChkBox(hdwnd, IDC_RO_ALP_TYPE_0, ro->alpType == 0);
	setDlgChkBox(hdwnd, IDC_RO_ALP_TYPE_1, ro->alpType == 1);
	setDlgChkBox(hdwnd, IDC_RO_ALP_TYPE_2, ro->alpType == 2);
	setDlgChkBox(hdwnd, IDC_RO_ALP_TYPE_3, ro->alpType == 3);

	return 0;
}


static int getDlgChkBox(HWND hdwnd, int idc)
{
	return (SendDlgItemMessage(hdwnd, idc, BM_GETCHECK, 0, 0) != BST_UNCHECKED);
}


static int setDlgChkBox(HWND hdwnd, int idc, int n)
{
	n = n ? BST_CHECKED : BST_UNCHECKED;
	return (int)SendDlgItemMessage(hdwnd, idc, BM_SETCHECK, n, 0);
}


static int dlgText2xval(HWND hdwnd, int idc)
{
	char buf[256];
	int n;

	GetDlgItemText(hdwnd, idc, (LPSTR)buf, sizeof(buf)-1);
	//sscanf(buf, "%x", &n);
	n = strtol(buf, 0, 16);
	return n;
}


static int dlgText2val(HWND hdwnd, int idc)
{
	char buf[256];
	int  n;

	GetDlgItemText(hdwnd, idc, (LPSTR)buf, sizeof(buf)-1);
	//sscanf(buf, "%d", &n);
	n = strtol(buf, 0, 10);
	return n;
}


static int xval2dlgText(HWND hdwnd, int idc, int val)
{
	char buf[16];

	sprintf(buf, "%06x", val);
	SetDlgItemText(hdwnd, idc, (LPSTR)buf);
	return 0;
}


static int val2dlgText(HWND hdwnd, int idc, int val)
{
	char buf[16];

	sprintf(buf, "%d", val);
	SetDlgItemText(hdwnd, idc, (LPSTR)buf);
	return 0;
}



/** BG1,2,grid線の色選択ダイアログ用の初期化
 */
static void chooseColor_init(HWND hWnd, COLORREF color)
{
	static COLORREF		custColors[16] = {
		0x444444, 0x888888, 0xCCCCCC, 0xFFFFFF,
		0x442222, 0x882222, 0xCC2222, 0xFF2222,
		0x224422, 0x228822, 0x22CC22, 0x22FF22,
		0x222244, 0x222288, 0x2222CC, 0x2222FF,
	};
	main_chooseColor.lStructSize	= sizeof (CHOOSECOLOR);
	main_chooseColor.hwndOwner		= hWnd;
	main_chooseColor.rgbResult		= color;
	main_chooseColor.lpCustColors	= custColors;
	main_chooseColor.Flags			= CC_FULLOPEN | CC_RGBINIT;
}



#define SWAP_R_B(c)		((BYTE)((c)>>16) | ((BYTE)(c)<<16) | ((c) & 0xFF00FF00))

/** BG1,2,grid線の色選択ダイアログ.
 */
static void setChooseColor(HWND hWnd, DWORD* pCol, int item) {
	if (main_hClutWnd)
		return;
	main_chooseColor.rgbResult	= SWAP_R_B(*pCol);
	if (ChooseColor(&main_chooseColor)) {
		char buf[128];
		*pCol = SWAP_R_B(main_chooseColor.rgbResult);
		sprintf(buf, "%x", *pCol);
		SetDlgItemText(hWnd, item, buf);
	}
}


/*-----------------------------------------------------------------------*/

/** 現在の内容をレジストリに記録.
 */
static int	RegOptPut(regopt_t *ro)
{
	HKEY	hKey;
	DWORD 	dwPosition;

	RegCreateKeyEx(HKEY_CURRENT_USER, REGOPT_FILE_NAME, 0, "",
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwPosition);
	RegSetValueEx(hKey, "ORDER", 0, REG_BINARY, (CONST BYTE *)ro, sizeof(*ro));
	RegCloseKey(hKey);
	return 1;
}


/** レジストリに記録している情報を取得.
 */
static int RegOptGet(regopt_t *ro)
{
	HKEY 	hKey;
	DWORD 	dwType 	= REG_BINARY;
	DWORD 	dwByte 	= sizeof(regopt_t);
	DWORD 	dwPosition;
	int 	n;

	n = RegOpenKeyEx(HKEY_CURRENT_USER, REGOPT_FILE_NAME, 0, KEY_ALL_ACCESS, &hKey);
	if (n != ERROR_SUCCESS) {
		n = RegCreateKeyEx(HKEY_CURRENT_USER, REGOPT_FILE_NAME, 0, "",
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwPosition);
		SetDfltOpt(ro);
		RegOptPut(ro);
		return 0;
	}
	if (n != ERROR_SUCCESS)
		return 0;
	memset(ro, 0, sizeof(*ro));
	n = RegQueryValueEx(hKey, "ORDER", NULL, &dwType, (BYTE *)ro, &dwByte);
	RegCloseKey(hKey);
	if (n != ERROR_SUCCESS && n != ERROR_MORE_DATA)
		return 0;
	return ro->enable;
}



/** デフォルト設定.
 */
static void SetDfltOpt(regopt_t *ro)
{
	memset(ro, 0, sizeof(*ro));
	ro->ver 		= 0x100;					// 上位WORD. 互換制を維持している間は固定の値. 下位 バージョン番号
	ro->size		= sizeof(*ro);				// この構造体のサイズ
	ro->enable		= 3;						// bit0:レジストリの内容を	  0=使わない 1=使う. bit1:画像情報の取得を要求	0=しない   1=する
	ro->bgGridMode	= 1;						// BG,grid処理を 0:しない  1:する
	ro->gridFlg 	= 0;						// グリッド線を 0:無し	1:描く
	ro->gridCol 	= GR_RGB(0xff,0xff,0xff);	// 色
	ro->bgCol1		= GR_RGB(0x80,0x80,0x80);	// BG色1
	ro->bgCol2		= GR_RGB(0xC0,0xC0,0xC0);	// BG色2
	ro->gridW		= 16;						// グリッドの横幅 2～1024
	ro->gridH		= 16;						// グリッドの縦幅 2～1024
	ro->tone		= 1000; 					// 輝度. 1000 を 100%とする
	ro->clutOfs 	= 0;						// clutテーブルをこの値分ずらして描画。
	ro->clutKey 	= 0;						// clutでの抜き色番号
	ro->colKey		= 0;						// 抜き色 RGB値。
	ro->alpVal		= 255;						// toneに統合。(ピクセル固有のα値をさらに256段階にする)

	ro->keyMode 	= 0;						// 抜色処理 0:無し	1:256色以下はclut番号,多色はダイレクトRGB  2:すべてダイレクトRGB
	ro->alpType 	= 1;						// α情報の扱い 0:α無視  1:ブレンド  2:加算  3:減算
	ro->alpMode 	= 0;						// 0:全体にα値を反映  1:α値が 0以外のピクセルのみに反映
	ro->alpHalf 	= 0;						// 0:通常 1:α値は 128が100%となるように扱う
	ro->alpRev		= 0;						// α値の扱いを 0:0xFFなら不透明～0なら透明  1:0なら不透明～0xFFなら透明
	ro->bgFileFlg	= 0;						// BG表示要求 0:しない	1:する
	ro->aspReq		= 0;						// 元画像のドット比が1:1でないとき調整 0:しない 2:する

	ro->mapOffSw	= 0;

	ro->bpp32use	= 0;
}




/*-----------------------------------------------------------------------*/

/** レジストリに記録された画像１枚の情報を取得.
 */
static int RegRsltGet(regopt_s_t *rs, DWORD *clut, char *fname)
{
	char		name[MAX_PATH*2+4];
	regopt_s_t	rst;
	DWORD		dwType;
	DWORD		dwByte;
	HKEY		hKey;
	int 		n;

	memset(&rst, 0, sizeof(rst));
	n = RegOpenKeyEx(HKEY_CURRENT_USER, REGOPT_FILE_NAME, 0, KEY_ALL_ACCESS, &hKey);
	if (n != ERROR_SUCCESS) {
		memset(rs, 0, sizeof(*rs));
		return 0;
	}

	dwType = REG_SZ;
	dwByte = sizeof(name)-2;
	n = RegQueryValueEx(hKey, "FNAME", NULL, &dwType, (BYTE *)name, &dwByte);
	if (n != ERROR_SUCCESS) {
		//RegCloseKey(hKey);
		//return 0;
		name[0] = 0;
	}

	dwType = REG_BINARY;
	dwByte = REGOPT_CLUT_NUM * 4;
	n = RegQueryValueEx(hKey, "CLUT", NULL, &dwType, (BYTE *)clut, &dwByte);
	if (n != ERROR_SUCCESS && n != ERROR_MORE_DATA) {
		memset(clut, 0, REGOPT_CLUT_NUM*4);
	}

	dwType = REG_BINARY;
	dwByte = sizeof(rst);
	n = RegQueryValueEx(hKey, "GINFO", NULL, &dwType, (BYTE *)&rst, &dwByte);
	if (n == ERROR_SUCCESS && memcmp(rs, &rst, sizeof(*rs)) == 0 && name[0] && stricmp(name, regrsltDat.regfname) == 0) {
		RegCloseKey(hKey);
		return 0;
	} else {
		if (name[0])
			strcpy(fname, name);
		memcpy(rs, &rst, sizeof(*rs));
	}
	RegCloseKey(hKey);
	return 1;
}


/** clut値の描画
 */
static void DrawClutData(regopt_s_t *rs, DWORD clut[])
{
	int 	x0=4, y0=4;
	int 	i,x,y,a,c;
	int 	md,md0,mdF;

	gr_cls(0x00/*regopt.bgCol1*/);

	md = md0 = mdF = 0;
	for ( i = 0; i < 256; i++) {
		a = clut[i] >> 24;
		if (a == 0) {
			md0++;
		} else if (a == 0xFF) {
			mdF++;
		} else {
			md++;
		}
	}

	md = (md0 != 256 && mdF != 256);

	if (rs->bpp <= 8) {
		for (i = 0; i < 256; i++) {
			x = i & 15;
			y = i >> 4;
			c = clut[i];
			if (md == 0) {
				gr_boxFill(x0+1+x*14, y0+1+y*14, 12, 12, c & 0xFFFFFF);
			} else {
				gr_boxFill(x0+1+x*14, y0+1+y*14, 12,  8, c & 0xFFFFFF);
				a = c >> 24;
				c = BBBB(0,a,a,a);
				gr_boxFill(x0+1+x*14, y0+1+y*14+9, 12, 3, c);
			}
		}
	}
}



/** clut表示.
 */
static void PaintClutStatus(regopt_s_t *rs, DWORD clut[])
{
	char	buf[256];
	char 	wk[256];
	int 	c;

	sprintf(buf, "%3dbit色 %4d*%-4d ", rs->bpp, rs->w, rs->h);
	if (rs->x0 || rs->y0) {
		sprintf(wk, "ofs(%d,%d) ", rs->x0, rs->y0);
		strcat(buf, wk);
	}
	c = rs->clutKey;
	if (c > 0 && c <= 256) {
		--c;
		sprintf(wk, "(透明=%d) ", c);
		strcat(buf, wk);
	} else if (rs->colorKey > 0 && rs->colorKey <= 0xFFFFFF+1) {
		c = rs->colorKey - 1;
		sprintf(wk, "(透明=%06x) ", c);
		strcat(buf, wk);
	}
	strcat(buf, fname_baseName(regrsltDat.regfname));
	SendMessage(main_hStatWnd, SB_SETTEXT, (WPARAM)0, (LPARAM)buf);
}




static char *fname_baseName(char *adr)
{
	char *p;

	if (adr == NULL)
		return "";
	p = adr;
	while (*p != '\0') {
		if (*p == ':' || *p == '/' || *p == '\\')
			adr = p + 1;
		if (_ismbblead(*p) && *(p+1) )
			p++;
		p++;
	}
	return adr;
}

