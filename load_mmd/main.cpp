#include"include/load_pmx.hpp"

int main()
{
	auto modelIf = ichi::load_pmx("../../mmd/Paimon/�h��.pmx");

	auto&& model = modelIf.value();
	
	return 0;
}