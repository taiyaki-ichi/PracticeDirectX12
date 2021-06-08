//#include"Window.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include<stb_image.h>

#include<utility>

int main()
{
	/*
	using namespace DX12;

	constexpr std::size_t WINDOW_WIDTH = 1024;
	constexpr std::size_t WINDOW_HEIGHT = 768;

	auto hwnd = CreateSimpleWindow(L"test", WINDOW_WIDTH, WINDOW_HEIGHT);

	while (UpdateWindow()) {};
	*/

	int x, y, n;
	std::uint8_t *data = stbi_load("image.png", &x, &y, &n, 0);


	return 0;
}