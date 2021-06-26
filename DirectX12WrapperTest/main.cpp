#include"test001.hpp"
//#include"test002.hpp"
#include"test003.hpp"
#include"test004.hpp"
#include"test005.hpp"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//return test001::main();
	//return test002::main();
	//return test003::main();
	//return test004::main();
	return test005::main();
}