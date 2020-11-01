#include"window.hpp"
#include<iostream>

int main()
{
	auto hwnd = ggg::create_window(L"aaaa", 400, 300);

	while (ggg::process_window_message());


	return 0;
}