#pragma once
#include"pmx_data_struct.hpp"
#include<variant>
#include<string>
#include<optional>

namespace ichi
{
	std::optional<std::variant<pmx_model<std::wstring>, pmx_model<std::string>>>
		load_pmx(const char* fileName);
}

