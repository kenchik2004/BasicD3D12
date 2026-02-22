#include "CommandQueue.h"

namespace System {

	CommandQueue::CommandQueue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
	{
		if (!device) return;

		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = type;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		HRESULT hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(command_queue.GetAddressOf()));
		if (hr != S_OK) return;

		hr = device->CreateFence(fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));
		if (hr != S_OK) {
			command_queue.Reset();
		}
	}

	CommandQueue::~CommandQueue()
	{
		WaitForCompletion();
		command_queue.Reset();
		fence.Reset();
	}

	int CommandQueue::Execute(UINT num_lists, ID3D12CommandList* const* command_lists)
	{
		if (!IsValid()) return -1;
		command_queue->ExecuteCommandLists(num_lists, command_lists);
		command_queue->Signal(fence.Get(), ++fence_value);
		return 0;
	}

	int CommandQueue::WaitForCompletion()
	{
		if (!fence) return -1;
		while (fence->GetCompletedValue() < fence_value)
		{
			// フェンスの完了値がfence_valueに達するまで待機する
		}
		return 0;
	}
}
