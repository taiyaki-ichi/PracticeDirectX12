#pragma once
#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<d3dcompiler.h>
#include<iostream>
#include<string>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

namespace ichi
{
	//ファイルネームとエントリポイントからシェーダをコンパイルしblobを返す
	ID3DBlob* create_shader_blob(const wchar_t* fileName, const char* funcName, const char* include)
	{
		ID3DBlob* blob = nullptr;
		ID3DBlob* errorBlob = nullptr;

		auto result = D3DCompileFromFile(
			fileName,
			nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			funcName, include,
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0, &blob, &errorBlob);

		//失敗した場合
		if (FAILED(result))
		{
			std::cout << __func__ << " is failed ";
			if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
				std::cout << " : file is not found\n";
			}
			else {
				std::string errstr;
				errstr.resize(errorBlob->GetBufferSize());
				std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
				std::cout << " : " << errstr << " : " << fileName << "\n";
			}

			return nullptr;
		}

		return blob;
	}
}