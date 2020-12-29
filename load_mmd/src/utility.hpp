#pragma once
#include<fstream>
#include<string>

namespace MMDL
{
	//ファイルからバイナリを読み込む
	template<typename T>
	inline void read_binary_from_file(std::ifstream& stream, T* value) {
		stream.read(reinterpret_cast<char*>(value), sizeof(*value));
	}
	template<typename T>
	inline void read_binary_from_file(std::ifstream& stream, T* value, size_t size) {
		stream.read(reinterpret_cast<char*>(value), size);
	}

	//stringからwstringへ
	std::wstring to_wstring(const std::string& str);
	//wstringからstringへ
	std::string to_string(const std::wstring& str);



}
