#pragma once
#include"Window.hpp"

namespace test011
{
	using namespace DX12;

	HWND CreateMyWindow(const wchar_t* window_name, std::uint32_t width, std::uint32_t height)
	{
		HINSTANCE hInstance = GetModuleHandle(nullptr);

		//ウィンドウクラス設定
		WNDCLASSEX wcex{};
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.lpfnWndProc = wnd_proc;
		wcex.lpszClassName = window_name;
		wcex.hInstance = hInstance;

		//ウィンドウクラスの登録
		RegisterClassEx(&wcex);

		//ウィンドウサイズの設定
		RECT rect{ 0,0,static_cast<LONG>(width),static_cast<LONG>(height) };

		HWND hwnd = CreateWindow(
			wcex.lpszClassName,			//クラス名
			window_name,				//タイトルバー
			WS_BORDER,//WS_OVERLAPPEDWINDOW,		//タイトルバーと境界線があるウィンドウ
			CW_USEDEFAULT,				//x座標
			CW_USEDEFAULT,				//y座標
			rect.right - rect.left,		//ヨコ
			rect.bottom - rect.top,		//タテ
			nullptr,					//親ウィンドウ
			nullptr,					//メニューハンドル
			wcex.hInstance,				//呼び出しアプリケーションハンドル
			nullptr						//追加パラメータ
		);

		//ウィンドウの表示
		ShowWindow(hwnd, SW_SHOW);

		return hwnd;
	}



	inline int main()
	{
		constexpr std::size_t WINDOW_WIDTH = 1024;
		constexpr std::size_t WINDOW_HEIGHT = 768;

		auto hwnd1 = CreateMyWindow(L"window1", WINDOW_WIDTH, WINDOW_HEIGHT);
		//auto hwnd2 = create_simple_window(L"window2", WINDOW_WIDTH, WINDOW_HEIGHT);

		system("Pause");

		return 0;
	}

}