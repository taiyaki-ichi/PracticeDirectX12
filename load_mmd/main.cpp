#include"load_pmx.hpp"

int main()
{
	ichi::load_pmx("../../mmd/Paimon/派蒙.pmx");
	//ichi::load_pmx("../../mmd/紫咲シオン公式mmd_ver1.01/紫咲シオン.pmx");
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