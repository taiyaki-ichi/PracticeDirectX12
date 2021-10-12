#pragma once
#include"Window.hpp"

namespace test011
{
	using namespace DX12;

	HWND CreateMyWindow(const wchar_t* window_name, std::uint32_t width, std::uint32_t height)
	{
		HINSTANCE hInstance = GetModuleHandle(nullptr);

		//�E�B���h�E�N���X�ݒ�
		WNDCLASSEX wcex{};
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.lpfnWndProc = wnd_proc;
		wcex.lpszClassName = window_name;
		wcex.hInstance = hInstance;

		//�E�B���h�E�N���X�̓o�^
		RegisterClassEx(&wcex);

		//�E�B���h�E�T�C�Y�̐ݒ�
		RECT rect{ 0,0,static_cast<LONG>(width),static_cast<LONG>(height) };

		HWND hwnd = CreateWindow(
			wcex.lpszClassName,			//�N���X��
			window_name,				//�^�C�g���o�[
			WS_BORDER,//WS_OVERLAPPEDWINDOW,		//�^�C�g���o�[�Ƌ��E��������E�B���h�E
			CW_USEDEFAULT,				//x���W
			CW_USEDEFAULT,				//y���W
			rect.right - rect.left,		//���R
			rect.bottom - rect.top,		//�^�e
			nullptr,					//�e�E�B���h�E
			nullptr,					//���j���[�n���h��
			wcex.hInstance,				//�Ăяo���A�v���P�[�V�����n���h��
			nullptr						//�ǉ��p�����[�^
		);

		//�E�B���h�E�̕\��
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