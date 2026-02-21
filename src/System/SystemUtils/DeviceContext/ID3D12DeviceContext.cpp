#include "ID3D12DeviceContext.h"


namespace System {
	ID3D12DeviceContext::ID3D12DeviceContext(ID3D12Device* master_device, D3D12_COMMAND_LIST_TYPE context_type)
	{
		if (!master_device) return;		// 引数のデバイスがnullptrだったら、何もせずに関数を終了する

		// コマンドアロケータの作成
		master_device->CreateCommandAllocator(context_type, IID_PPV_ARGS(command_allocator.GetAddressOf()));
		// コマンドリストの作成
		master_device->CreateCommandList(0, context_type, command_allocator.Get(), nullptr, IID_PPV_ARGS(command_list.GetAddressOf()));

		// コマンドリストは、作成した直後は「記録中」の状態になっているため、コマンドリストを閉じておく
		command_list->Close();
	}
}