//#include"test001.hpp"
//#include"test002.hpp"
//#include"test003.hpp"
#include"test004.hpp"
//#include"test005.hpp"
//#include"test006.hpp"
//#include"test007.hpp"
//#include"test008.hpp"
//#include"test009.hpp"
//#include"test010.hpp"


#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>





int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//return test001::main();
	//return test002::main();
	//return test003::main();
	return test004::main();
	//return test005::main();
	//return test006::main();
	//return test007::main();
	//return test008::main();
	//return test009::main();
	//return test010::main();
}