#include"../include/load_pmx.hpp"
#include<string>
#include<fstream>
#include<iostream>
#include"load_pmx_impl.hpp"


namespace ichi
{


	std::optional<std::variant<pmx_model<std::wstring>, pmx_model<std::string>>>
		load_pmx(const char* fileName) {

		std::ifstream file{ fileName, std::ios::in | std::ios::binary };

		if (!file) {
			std::cout << "fopen is failed\n";
			return std::nullopt;
		}


		//ƒwƒbƒ_
		pmx_header header{};
		{
			//Å‰‚Ì"Pmx "‚Ì•¶Žš
			//Žg‚í‚È‚¢‚Ì‚Å”jŠü
			unsigned char pmx[4];
			read_binary_from_file(file, &pmx[0], 4);

			read_binary_from_file(file, &header.m_version);
			read_binary_from_file(file, &header.m_data_size);
			read_binary_from_file(file, &header.m_data[0], header.m_data_size);
		}

		//UTF16
		if (header.m_data[pmx_header::data_index::ENCODING_FORMAT] == 0)
		{
			auto&& [modelInfo, vertex, surface, texturePath, material, born]
				= load_pmx_impl<std::wstring>(file, header, fileName);

			return pmx_model<std::wstring>{
				std::move(header),
				std::move(modelInfo),
				std::move(vertex),
				std::move(surface),
				std::move(texturePath),
				std::move(material),
				std::move(born)
				};
		}
		//UTF8
		else if (header.m_data[pmx_header::data_index::ENCODING_FORMAT] == 0)
		{
			auto&& [modelInfo, vertex, surface, texturePath, material, born]
				= load_pmx_impl<std::string>(file, header, fileName);

			return pmx_model<std::string>{
				std::move(header),
					std::move(modelInfo),
					std::move(vertex),
					std::move(surface),
					std::move(texturePath),
					std::move(material),
					std::move(born)
			};

		}

		return std::nullopt;
	
	}

}