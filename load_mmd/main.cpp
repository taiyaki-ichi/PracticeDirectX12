#include"load_pmx.hpp"

int main()
{
	ichi::load_pmx("../../mmd/Paimon/�h��.pmx");
	//ichi::load_pmx("../../mmd/����V�I������mmd_ver1.01/����V�I��.pmx");
	/*
	FILE* gnu = _popen("gnuplot", "w");
	fprintf(gnu, "splot '-' with points pt 7 ps 1\n");
	for (auto& tmp : v) {
		fprintf(gnu, "%f %f %f\n", tmp.x, tmp.y, tmp.z);
	}
	fprintf(gnu, "e\n");

	fflush(gnu);

	system("pause");

	_pclose(gnu);

	*/
	return 0;
}