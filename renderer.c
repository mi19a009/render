/* Copyright 2026 Taichi Murakami. */
#include "stdafx.h"
#include "renderer.h"
#include "resource.h"
#define DEFPROC DefWindowProc((hWnd), (uMsg), (wParam), (lParam))
#define DEFAULT_BACKGROUND RGB(32, 64, 128)
#define CUSTCOLORCOUNT 16

/* User Data */
typedef struct tagRENDERERUSERDATA
{
	COLORREF rgbCustColors[CUSTCOLORCOUNT];
} USERDATA, FAR *LPUSERDATA;

/* Window Extra */
typedef struct tagRENDERERWINDOWEXTRA
{
	DWORD_PTR hglrc;
	DWORD rgbBackground;
} WINDOWEXTRA;

STATIC_ASSERT(sizeof(WINDOWEXTRA) == RENDERERWINDOWEXTRA);
#define GWLP_HGLRC              offsetof(WINDOWEXTRA, hglrc)
#define GWL_BACKGROUND          offsetof(WINDOWEXTRA, rgbBackground)

static void WINAPI
ClearColor(_In_ COLORREF rgb);

static void WINAPI
DestroyContext(_In_ HWND hWnd);

static void WINAPI
DestroyUserData(_In_ HWND hWnd);

static BOOL WINAPI
InitPixelFormat(_In_ HDC hDC);

static void WINAPI
OnAbout(_In_ HWND hWnd);

static void WINAPI
OnBackground(_In_ HWND hWnd);

static BOOL WINAPI
OnCreate(_In_ HWND hWnd);

static void WINAPI
OnDestroy(_In_ HWND hWnd);

static void WINAPI
OnPaint(_In_ HWND hWnd);

/**
* @brief OpenGL 背景色を設定します。
*/
static void WINAPI
ClearColor(_In_ COLORREF rgb)
{
	FLOAT r, g, b;
	r = GetRValue(rgb) / 255.0F;
	g = GetGValue(rgb) / 255.0F;
	b = GetBValue(rgb) / 255.0F;
	glClearColor(r, g, b, 1.0F);
}

/**
* @brief OpenGL コンテキストを破棄します。
*/
static void WINAPI
DestroyContext(_In_ HWND hWnd)
{
	HGLRC hGL;
	hGL = (HGLRC)SetWindowLongPtr(hWnd, GWLP_HGLRC, 0);

	if (hGL)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hGL);
	}
}

/**
* @brief カスタム色を破棄します。
*/
static void WINAPI
DestroyUserData(_In_ HWND hWnd)
{
	LPUSERDATA lpUserData;
	lpUserData = (LPUSERDATA)SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);

	if (lpUserData)
	{
		LocalFree(lpUserData);
	}
}

/**
* @brief OpenGL ピクセル書式を用意します。
*/
static BOOL WINAPI
InitPixelFormat(_In_ HDC hDC)
{
	PIXELFORMATDESCRIPTOR pfd;
	int nResult;
	ZeroMemory(&pfd, sizeof pfd);
	pfd.nSize = sizeof pfd;
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	nResult = ChoosePixelFormat(hDC, &pfd);

	if (nResult)
	{
		nResult = SetPixelFormat(hDC, nResult, &pfd);
	}

	return nResult;
}

/**
* @brief バージョン情報ダイアログを表示します。
*/
static void WINAPI
OnAbout(_In_ HWND hWnd)
{
	HINSTANCE hInstance;
	hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutDialogProc);
}

/**
* @brief 背景色を選択します。
*/
static void WINAPI
OnBackground(_In_ HWND hWnd)
{
	CHOOSECOLOR cc;
	ZeroMemory(&cc, sizeof cc);
	cc.lStructSize = sizeof cc;
	cc.hwndOwner = hWnd;
	cc.rgbResult = GetWindowLong(hWnd, GWL_BACKGROUND);
	cc.lpCustColors = ((LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA))->rgbCustColors;
	cc.Flags = CC_RGBINIT;

	if (ChooseColor(&cc))
	{
		SetWindowLong(hWnd, GWL_BACKGROUND, cc.rgbResult);
		InvalidateRect(hWnd, NULL, TRUE);
	}
}

/**
* @brief 指定したメニュー項目を実行します。
*/
static BOOL WINAPI
OnCommand(_In_ HWND hWnd, _In_ UINT idMenu)
{
	BOOL bResult = FALSE;

	switch (idMenu)
	{
	case IDM_ABOUT:
		OnAbout(hWnd);
		break;
	case IDM_BACKGROUND:
		OnBackground(hWnd);
		break;
	case IDM_EXIT:
		SendMessage(hWnd, WM_CLOSE, 0, 0);
		break;
	default:
		bResult = TRUE;
		break;
	}

	return bResult;
}

/**
* @brief ウィンドウを作成します。
*/
static BOOL WINAPI
OnCreate(_In_ HWND hWnd)
{
	LPUSERDATA lpUserData;
	BOOL bResult;
	lpUserData = LocalAlloc(LMEM_FIXED, sizeof(USERDATA));

	if (lpUserData)
	{
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)lpUserData);
		SetWindowLong(hWnd, GWL_BACKGROUND, DEFAULT_BACKGROUND);
		bResult = TRUE;
	}
	else
	{
		bResult = FALSE;
	}

	return bResult;
}

/**
* @brief アプリケーションを終了します。
*/
static void WINAPI
OnDestroy(_In_ HWND hWnd)
{
	DestroyContext(hWnd);
	DestroyUserData(hWnd);
	PostQuitMessage(0);
}

/**
* @brief クライアント領域を描画します。
*/
static void WINAPI
OnPaint(_In_ HWND hWnd)
{
	HDC hDC;
	HGLRC hGL;
	PAINTSTRUCT paint;
	hDC = BeginPaint(hWnd, &paint);

	if (hDC)
	{
		hGL = (HGLRC)GetWindowLongPtr(hWnd, GWLP_HGLRC);

		if (!hGL && InitPixelFormat(hDC))
		{
			hGL = wglCreateContext(hDC);
			SetWindowLongPtr(hWnd, GWLP_HGLRC, (LONG_PTR)hGL);
		}
		if (hGL)
		{
			wglMakeCurrent(hDC, hGL);
			ClearColor(GetWindowLong(hWnd, GWL_BACKGROUND));
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			SwapBuffers(hDC);
		}

		EndPaint(hWnd, &paint);
	}
}

/**
* @brief アプリケーション ウィンドウ プロシージャです。
*/
LRESULT CALLBACK
RendererWindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	LRESULT nResult;

	switch (uMsg)
	{
	case WM_COMMAND:
		if (OnCommand(hWnd, LOWORD(wParam))) nResult = DEFPROC;
		else nResult = 0;
		break;
	case WM_CREATE:
		if (OnCreate(hWnd)) nResult = DEFPROC;
		else nResult = -1;
		break;
	case WM_DESTROY:
		OnDestroy(hWnd);
		nResult = DEFPROC;
		break;
	case WM_ERASEBKGND:
		nResult = TRUE;
		break;
	case WM_PAINT:
		OnPaint(hWnd);
		nResult = 0;
		break;
	default:
		nResult = DEFPROC;
		break;
	}

	return nResult;
}
