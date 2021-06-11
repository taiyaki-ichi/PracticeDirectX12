#pragma once
#include<vector>
#include<fstream>
#include<string>
#include<type_traits>
#include<sstream>

namespace OffLoader
{
	template<typename VertexType, typename FaceType>
	inline std::pair<std::vector<VertexType>, std::vector<FaceType>> LoadTriangularMeshFromOffFile(const char* fileName);

	inline void GetValidLine(std::ifstream&, std::string&);

	template<typename Container>
	void GetElement(std::string& line,Container&);

	//
	//
	//

	template<typename VertexType,typename FaceType>
	inline std::pair<std::vector<VertexType>, std::vector<FaceType>> LoadTriangularMeshFromOffFile(const char* fileName)
	{
		std::ifstream is{ fileName };
		if (!is)
			throw "";
		
		char magic[3];
		is.read(magic, sizeof(magic));
		if (!std::equal(std::begin(magic), std::end(magic), "OFF"))
			throw "";

		std::size_t vertexSize, faceSize, hoge;
		is >> vertexSize;
		is >> faceSize;
		//
		is >> hoge;

		std::string line;

		std::vector<VertexType> vertexList{};
		std::array<double, 3> tmpVertexElement;
		for (std::size_t i = 0; i < vertexSize; i++) {
			GetValidLine(is, line);
			GetElement(line, tmpVertexElement);
			vertexList.push_back({ tmpVertexElement[0], tmpVertexElement[1], tmpVertexElement[2] });
		}

		std::vector<FaceType> faceList{};
		std::array<std::size_t, 4> tmpFaceElement;
		faceList.reserve(faceSize);
		for (std::size_t i = 0; i < faceSize; i++) {

			GetValidLine(is, line);
			GetElement(line, tmpFaceElement);

			if (tmpFaceElement[0] != 3)
				throw "";

			faceList.push_back({ tmpFaceElement[1], tmpFaceElement[2], tmpFaceElement[3] });
		}

		return { std::move(vertexList),std::move(faceList) };
	}

	inline void GetValidLine(std::ifstream& is, std::string& line)
	{
		while (std::getline(is, line)) {
			if (line.size() > 1 && line[0] != '#')
				break;
		}
	}
	template<typename Container>
	void GetElement(std::string& line, Container& c)
	{
		std::stringstream ss{ line };
		for (std::size_t i = 0; i < c.size(); i++)
			ss >> c[i];
	}
}