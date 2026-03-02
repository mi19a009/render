/* Copyright (C) 2026 Taichi Murakami. */
#include "stdafx.h"
#include "renderer.h"
#include "resource.h"
#define CUSTCOLORCOUNT          16
#define DEFAULT_BACKGROUND      RGB(32, 64, 128)
#define DEFAULT_DISTANCE        5
#define DEFAULT_FIELDOFVIEW     2
#define DEFAULT_ZFAR            100
#define DEFAULT_ZNEAR           3
#define DEFPROC                 DefWindowProc((hWnd), (uMsg), (wParam), (lParam))
#define ROTATION_INCREMENT      17
#define ROTATION_MAX            360
#define TIMER_ELAPSE            100
#define TIMER_ID                1

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
	DWORD nViewportWidth;
	DWORD nViewportHeight;
	WORD nRotation;
	WORD nFieldOfView;
	WORD zNear;
	WORD zFar;
	WORD nDistance;
	WORD nReserved;
} WINDOWEXTRA;

STATIC_ASSERT(sizeof(WINDOWEXTRA) == RENDERERWINDOWEXTRA);
#define GWLP_HGLRC              offsetof(WINDOWEXTRA, hglrc)
#define GWL_BACKGROUND          offsetof(WINDOWEXTRA, rgbBackground)
#define GWL_VIEWPORTWIDTH       offsetof(WINDOWEXTRA, nViewportWidth)
#define GWL_VIEWPORTHEIGHT      offsetof(WINDOWEXTRA, nViewportHeight)
#define GWW_ROTATION            offsetof(WINDOWEXTRA, nRotation)
#define GWW_FIELDOFVIEW         offsetof(WINDOWEXTRA, nFieldOfView)
#define GWW_ZNEAR               offsetof(WINDOWEXTRA, zNear)
#define GWW_ZFAR                offsetof(WINDOWEXTRA, zFar)
#define GWW_DISTANCE            offsetof(WINDOWEXTRA, nDistance)

static void WINAPI
DestroyContext(_In_ HWND hWnd);

static void WINAPI
DestroyUserData(_In_ HWND hWnd);

static BOOL WINAPI
InitPixelFormat(_In_ HDC hDC);

static void WINAPI
UpdateColor(_In_ HWND hWnd);

static void WINAPI
UpdateProjection(_In_ HWND hWnd);

static void WINAPI
UpdateViewport(_In_ HWND hWnd);

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

static void WINAPI
OnSize(_In_ HWND hWnd);

static void WINAPI
OnTimer(_In_ HWND hWnd);

static const VECTOR3 CubeMeshNormalBuffer[] =
{
	/* X */
	{ +1.0F, 0.0F, 0.0F },
	{ +1.0F, 0.0F, 0.0F },
	{ +1.0F, 0.0F, 0.0F },
	{ +1.0F, 0.0F, 0.0F },
	{ -1.0F, 0.0F, 0.0F },
	{ -1.0F, 0.0F, 0.0F },
	{ -1.0F, 0.0F, 0.0F },
	{ -1.0F, 0.0F, 0.0F },
	/* Y */
	{ 0.0F, +1.0F, 0.0F },
	{ 0.0F, +1.0F, 0.0F },
	{ 0.0F, +1.0F, 0.0F },
	{ 0.0F, +1.0F, 0.0F },
	{ 0.0F, -1.0F, 0.0F },
	{ 0.0F, -1.0F, 0.0F },
	{ 0.0F, -1.0F, 0.0F },
	{ 0.0F, -1.0F, 0.0F },
	/* Z */
	{ 0.0F, 0.0F, +1.0F },
	{ 0.0F, 0.0F, +1.0F },
	{ 0.0F, 0.0F, +1.0F },
	{ 0.0F, 0.0F, +1.0F },
	{ 0.0F, 0.0F, -1.0F },
	{ 0.0F, 0.0F, -1.0F },
	{ 0.0F, 0.0F, -1.0F },
	{ 0.0F, 0.0F, -1.0F },
};

/* 頂点バッファ */
static const VECTOR3 CubeMeshVertexBuffer[] =
{
	/* X */
	{ +1.0F, +1.0F, +1.0F },
	{ +1.0F, -1.0F, +1.0F },
	{ +1.0F, -1.0F, -1.0F },
	{ +1.0F, +1.0F, -1.0F },
	{ -1.0F, +1.0F, -1.0F },
	{ -1.0F, -1.0F, -1.0F },
	{ -1.0F, -1.0F, +1.0F },
	{ -1.0F, +1.0F, +1.0F },
	/* Y */
	{ +1.0F, +1.0F, +1.0F },
	{ +1.0F, +1.0F, -1.0F },
	{ -1.0F, +1.0F, -1.0F },
	{ -1.0F, +1.0F, +1.0F },
	{ -1.0F, -1.0F, +1.0F },
	{ -1.0F, -1.0F, -1.0F },
	{ +1.0F, -1.0F, -1.0F },
	{ +1.0F, -1.0F, +1.0F },
	/* Z */
	{ -1.0F, +1.0F, +1.0F },
	{ -1.0F, -1.0F, +1.0F },
	{ +1.0F, -1.0F, +1.0F },
	{ +1.0F, +1.0F, +1.0F },
	{ +1.0F, +1.0F, -1.0F },
	{ +1.0F, -1.0F, -1.0F },
	{ -1.0F, -1.0F, -1.0F },
	{ -1.0F, +1.0F, -1.0F },
};

/* 目録バッファ */
static const WORD CubeMeshIndexBuffer[] =
{
	/* X */
	0, 1, 2,
	2, 3, 0,
	4, 5, 6,
	6, 7, 4,
	/* Y */
	8, 9, 10,
	10, 11, 8,
	12, 13, 14,
	14, 15, 12,
	/* Z */
	16, 17, 18,
	18, 19, 16,
	20, 21, 22,
	22, 23, 20,
};

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
* @brief OpenGL 背景色を設定します。
*/
static void WINAPI
UpdateColor(_In_ HWND hWnd)
{
	COLORREF rgb;
	FLOAT r, g, b;
	rgb = GetWindowLong(hWnd, GWL_BACKGROUND);
	r = GetRValue(rgb) / 255.0F;
	g = GetGValue(rgb) / 255.0F;
	b = GetBValue(rgb) / 255.0F;
	glClearColor(r, g, b, 1.0F);
}

static void WINAPI
UpdateModelView(_In_ HWND hWnd)
{
	FLOAT x;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	x = GetWindowWord(hWnd, GWW_DISTANCE);
	glTranslatef(0.0F, 0.0F, -x);
	x = GetWindowWord(hWnd, GWW_ROTATION);
	glRotatef(x, 0.0F, 1.0F, 0.0F);
}

/**
* @brief 透視変換行列を更新します。
*/
static void WINAPI
UpdateProjection(_In_ HWND hWnd)
{
	FLOAT x, y, zNear, zFar, fov;
	x = (FLOAT)GetWindowLong(hWnd, GWL_VIEWPORTWIDTH);
	y = (FLOAT)GetWindowLong(hWnd, GWL_VIEWPORTHEIGHT);
	zNear = GetWindowWord(hWnd, GWW_ZNEAR);
	zFar = GetWindowWord(hWnd, GWW_ZFAR);
	fov = GetWindowWord(hWnd, GWW_FIELDOFVIEW);
	x = x / y;
	y = 1.0F;
	fov /= zNear;
	x /= fov;
	y /= fov;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-x, x, -y, y, zNear, zFar);
}

/**
* @brief ビューポートの大きさを更新します。
*/
static void WINAPI
UpdateViewport(_In_ HWND hWnd)
{
	LONG nWidth, nHeight;
	nWidth = GetWindowLong(hWnd, GWL_VIEWPORTWIDTH);
	nHeight = GetWindowLong(hWnd, GWL_VIEWPORTHEIGHT);
	glViewport(0, 0, nWidth, nHeight);
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
		SetWindowWord(hWnd, GWW_DISTANCE, DEFAULT_DISTANCE);
		SetWindowWord(hWnd, GWW_FIELDOFVIEW, DEFAULT_FIELDOFVIEW);
		SetWindowWord(hWnd, GWW_ZNEAR, DEFAULT_ZNEAR);
		SetWindowWord(hWnd, GWW_ZFAR, DEFAULT_ZFAR);
		SetTimer(hWnd, TIMER_ID, TIMER_ELAPSE, NULL);
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
	const FLOAT color[] = { 0.5F, 0.5F, 0.5F, 1.0F };
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
			UpdateColor(hWnd);
			UpdateViewport(hWnd);
			UpdateProjection(hWnd);
			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_LIGHT0);
			glEnable(GL_LIGHTING);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			UpdateModelView(hWnd);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, CubeMeshVertexBuffer);
			glNormalPointer(GL_FLOAT, 0, CubeMeshNormalBuffer);
			glDrawElements(GL_TRIANGLES, ARRAYSIZE(CubeMeshIndexBuffer), GL_UNSIGNED_SHORT, CubeMeshIndexBuffer);
			SwapBuffers(hDC);
		}

		EndPaint(hWnd, &paint);
	}
}

/**
* @brief ビューポートの大きさを更新します。
*/
static void WINAPI
OnSize(_In_ HWND hWnd)
{
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	SetWindowLong(hWnd, GWL_VIEWPORTWIDTH, rcClient.right);
	SetWindowLong(hWnd, GWL_VIEWPORTHEIGHT, rcClient.bottom);
}

/**
* @brief 回転角度を加算します。
*/
static void WINAPI
OnTimer(_In_ HWND hWnd)
{
	UINT nRotation;
	nRotation = GetWindowWord(hWnd, GWW_ROTATION);
	nRotation += ROTATION_INCREMENT;
	nRotation %= ROTATION_MAX;
	SetWindowWord(hWnd, GWW_ROTATION, nRotation);
	InvalidateRect(hWnd, NULL, TRUE);
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
	case WM_SIZE:
		OnSize(hWnd);
		nResult = DEFPROC;
		break;
	case WM_TIMER:
		OnTimer(hWnd);
		nResult = 0;
		break;
	default:
		nResult = DEFPROC;
		break;
	}

	return nResult;
}
