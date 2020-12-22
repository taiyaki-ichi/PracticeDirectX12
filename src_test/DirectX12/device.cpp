#include"device.hpp"
#include"command_list.hpp"
#include"double_buffer.hpp"
#include"pipeline_state.hpp"
#include<array>

#include<iostream>

namespace ichi
{

	device::~device() {
		if (m_device)
			m_device->Release();
		if (m_factory)
			m_factory->Release();
		if (m_adaptor)
			m_adaptor->Release();
	}


	bool device::initialize()
	{

		//�f�o�b�N
		ID3D12Debug* debugLayer = nullptr;
		if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer)))) {
			std::cout << "D3D12GetDebugInterface is failed\n";
			return false;
		}
		else {
			debugLayer->EnableDebugLayer();
			debugLayer->Release();
		}

		//�t�@�N�g��
		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&m_factory)))) {
			std::cout << "CreateDXGIFactory1 is failed\n";
			return false;
		}

		//�A�_�v�^�[
		UINT adapterIndex = 0;
		DXGI_ADAPTER_DESC1 desc{};
		while (true) {
			m_factory->EnumAdapters1(adapterIndex, &m_adaptor);
			m_adaptor->GetDesc1(&desc);

			//�K�؂ȃA�_�v�^�����������ꍇ
			if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
				break;

			adapterIndex++;
			//������Ȃ������ꍇ
			if (adapterIndex == DXGI_ERROR_NOT_FOUND) {
				std::cout << "adaptor is not found\n";
				return false;
			}
		}


		//�t�B�[�`�����x����񋓂��Ă���
		std::array levels{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};

		//�f�o�C�X�̍쐻
		for (auto l : levels) {
			//�K�؂ɐ����ł����ꍇ
			if (SUCCEEDED(D3D12CreateDevice(m_adaptor, l, IID_PPV_ARGS(&m_device))))
				return true;
		}
		//���s�����Ƃ�
		std::cout << "D3D12CreateDevice is failed\n";

		return false;
	}

	command_list* device::create_command_list()
	{
		auto commandList = new command_list{};
		if (commandList->initialize(this))
			return commandList;
		else {
			std::cout << "create_commandlist is failed\n";
			delete commandList;
			return nullptr;
		}
	}

	double_buffer* device::create_double_buffer(HWND hwnd, command_list* cl)
	{
		auto doubleBuffer = new double_buffer{};
		if (doubleBuffer->initialize(this, hwnd, cl)) {
			return doubleBuffer;
		}
		else {
			std::cout << "create_double_buffer is failed\n";
			delete doubleBuffer;
			return nullptr;
		}	
	}

	pipeline_state* device::create_pipline_state(ID3DBlob* vertexShader, ID3DBlob* pixelShader)
	{
		auto pipelineState = new pipeline_state{};
		if (pipelineState->initialize(this,vertexShader,pixelShader)) {
			return pipelineState;
		}
		else {
			std::cout << "create_pipline_state is failed\n";
			delete pipelineState;
			return nullptr;
		}
	}

	ID3D12Device* device::get()
	{
		return m_device;
	}

}