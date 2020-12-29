#include"utility.hpp"
#include<Windows.h>

namespace MMDL
{


	//string‚©‚çwstring‚Ö
	std::wstring to_wstring(const std::string& str)
	{
		if (str.empty())
			return {};

		int neededSize = MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), NULL, 0);
		std::wstring result(neededSize, 0);
		MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &result[0], neededSize);

		return result;
	}

	//wstring‚©‚çstring‚Ö
	std::string to_string(const std::wstring& str)
	{
		if (str.empty())
			return {};

		int neededSize = WideCharToMultiByte(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), NULL, 0, NULL, NULL);
		std::string result(neededSize, 0);
		WideCharToMultiByte(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &result[0], neededSize, NULL, NULL);

		return result;
	}

}