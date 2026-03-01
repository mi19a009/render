/* Copyright 2026 Taichi Murakami. */
#include "stdafx.h"
#include "renderer.h"
#include "resource.h"

static HWND WINAPI
CreateRenderer(_In_opt_ HINSTANCE hInstance);

static BOOL WINAPI
RegisterRendererClass(_In_opt_ HINSTANCE hInstance);

/**
* @brief アプリケーションのメイン エントリ ポイントです。
*/
int PASCAL
wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInst, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	HACCEL hAccel;
	HWND hWnd;
	MSG msg;
	int nExitCode = -1;

	if (RegisterRendererClass(hInstance))
	{
		hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_RENDERER));

		if (hAccel)
		{
			hWnd = CreateRenderer(hInstance);

			if (hWnd)
			{
				ShowWindow(hWnd, nCmdShow);
				UpdateWindow(hWnd);

				while (GetMessage(&msg, NULL, 0, 0) > 0)
				{
					if (!TranslateAccelerator(hWnd, hAccel, &msg))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}

				nExitCode = (int)msg.wParam;
			}

			DestroyAcceleratorTable(hAccel);
		}
	}

	return nExitCode;
}

/**
* @brief アプリケーション ウィンドウを作成します。
*/
static HWND WINAPI
CreateRenderer(_In_opt_ HINSTANCE hInstance)
{
	TCHAR strTitle[16];
	LoadString(hInstance, IDS_RENDERER, strTitle, ARRAYSIZE(strTitle));
	return CreateWindow(RENDERERCLASSNAME, strTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
}

/**
* @brief アプリケーション ウィンドウ クラスを登録します。
*/
static BOOL WINAPI
RegisterRendererClass(_In_opt_ HINSTANCE hInstance)
{
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof wc);
	wc.cbSize = sizeof wc;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = RendererWindowProc;
	wc.cbWndExtra = RENDERERWINDOWEXTRA;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_RENDERER);
	wc.lpszClassName = RENDERERCLASSNAME;
	return RegisterClassEx(&wc);
}

/**
* @brief バージョン情報ダイアログ プロシージャです。
*/
INT_PTR CALLBACK
AboutDialogProc(_In_ HWND hDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	INT_PTR bResult = 0;

	switch (uMsg)
	{
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			break;
		case IDOK:
			EndDialog(hDlg, IDOK);
			break;
		default:
			break;
		}

		break;
	case WM_INITDIALOG:
		bResult = TRUE;
		break;
	default:
		break;
	}

	return bResult;
}
