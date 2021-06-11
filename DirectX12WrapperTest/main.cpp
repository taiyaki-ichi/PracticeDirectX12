#include"test001.hpp"
#include"test002.hpp"

#include"OffLoader.hpp"

struct Vertex {
	double x, y, z;
};

struct Face {
	std::size_t v1, v2, v3;
};

int main()
{
	auto [vs, fs] = OffLoader::LoadTriangularMeshFromOffFile<Vertex, Face>("Assets/tetra.off");


	return test001::main();
	//return test002::main();
}