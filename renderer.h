/* Copyright 2026 Taichi Murakami. */
#pragma once
#include "stdafx.h"
#define STATIC_ASSERT(x) static_assert((x), #x)
#define RENDERERCLASSNAME TEXT("Renderer")
#ifdef _WIN64
#define RENDERERWINDOWEXTRA 16
#else
#define RENDERERWINDOWEXTRA 8
#endif

INT_PTR CALLBACK
AboutDialogProc(_In_ HWND hDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

LRESULT CALLBACK
RendererWindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
