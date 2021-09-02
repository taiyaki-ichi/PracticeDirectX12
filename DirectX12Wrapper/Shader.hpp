#pragma once
#include"Utility.hpp"
#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<d3dcompiler.h>
#include<string>
#include<Windows.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

namespace DX12
{
	class Shader
	{
		release_unique_ptr<ID3DBlob> blob_ptr{};

	public:
		Shader() = default;
		~Shader() = default;

		Shader(Shader&&) = default;
		Shader& operator=(Shader&&) = default;

		//ファイル名、関数名、インクルードオプション
		void Intialize(const wchar_t* fileName, const char* funcName, const char* includeOption);

		ID3DBlob* Get() const noexcept;
	};

	//
	//
	//

	inline void Shader::Intialize(const wchar_t* fileName, const char* funcName, const char* includeOption)
	{
		ID3DBlob* errorBlob = nullptr;

		ID3DBlob* tmp = nullptr;
		auto result = D3DCompileFromFile(
			fileName,
			nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			funcName, includeOption,
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0, &tmp, &errorBlob);

		if (FAILED(result))
		{
			if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
				THROW_EXCEPTION("file not found");
			}
			else {
				std::string errstr;
				errstr.resize(errorBlob->GetBufferSize());
				std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
				THROW_EXCEPTION(errstr.data());
			}
		}
		blob_ptr.reset(tmp);
	}

	inline ID3DBlob* Shader::Get() const noexcept
	{
		return blob_ptr.get();
	}

}