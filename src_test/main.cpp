#include"window.hpp"

int main()
{

	auto hwnd = ichi::create_window(L"aaaaa", 400.f, 400.f);
	while (ichi::update_window()) {

	}

	return 0;
}