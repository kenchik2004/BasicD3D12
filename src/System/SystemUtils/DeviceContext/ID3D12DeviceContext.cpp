#include "ID3D12DeviceContext.h"
#include "System/SystemUtils/D3DBuffer/D3DBufferInclude.h"
#include "System/SystemUtils/Descriptors/View/View.h"
#include "System/SystemUtils/DescriptorHeaps/DescriptorHeap/DescriptorHeap.h"


namespace System {
	ID3D12DeviceContext::ID3D12DeviceContext(ID3D12Device* master_device, D3D12_COMMAND_LIST_TYPE context_type)
	{
		if (!master_device) return;		// 引数のデバイスがnullptrだったら、何もせずに関数を終了する

		// コマンドアロケータの作成
		master_device->CreateCommandAllocator(context_type, IID_PPV_ARGS(command_allocator.GetAddressOf()));
		// コマンドリストの作成
		master_device->CreateCommandList(0, context_type, command_allocator.Get(), nullptr, IID_PPV_ARGS(command_list.GetAddressOf()));

		// コマンドリストは、作成した直後は「記録中」の状態になっているため、コマンドリストを閉じておく
		CloseCommandList();
	}
	int ID3D12DeviceContext::SetTextureBindless(Texture* texture, unsigned int slot)
	{
		return -1;
	}
	int ID3D12DeviceContext::SetConstantBuffer(ConstantBuffer* cb, unsigned int slot)
	{
		if (slot >= 16) return -1;	// スロット番号が16以上だったら、-1を返す
		if (!cb || !cb->IsValid()) return -1; // CBVがnullptrだったら、-1を返す
		command_list->SetGraphicsRootDescriptorTable(1 + slot, cb->Cbv()->GetGPUHandle());
		return 0;
	}
}