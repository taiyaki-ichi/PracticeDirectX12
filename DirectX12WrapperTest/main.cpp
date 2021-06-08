#include"Window.hpp"
#include"Device.hpp"
#include"CommandList.hpp"
#include"DoubleBuffer.hpp"


int main()
{
	using namespace DX12;

	constexpr std::size_t WINDOW_WIDTH = 1024;
	constexpr std::size_t WINDOW_HEIGHT = 768;

	auto hwnd = CreateSimpleWindow(L"test", WINDOW_WIDTH, WINDOW_HEIGHT);

	Device device{};
	device.Initialize();

	CommandList commandList{};
	commandList.Initialize(&device);

	DoubleBuffer doubleBuffer{};
	auto [factry, swapChain] = commandList.CreateFactryAndSwapChain(hwnd);
	doubleBuffer.Initialize(&device, factry, swapChain);



	while (UpdateWindow()) {
		doubleBuffer.BarriorToBackbuffer(&commandList, ResourceState::RenderTarget);
		doubleBuffer.ClearBackBuffer(&commandList);



		doubleBuffer.BarriorToBackbuffer(&commandList, ResourceState::Common);

		commandList.Close();
		commandList.Execute();
		commandList.Clear();

		doubleBuffer.Flip();
	};
	
	return 0;
}