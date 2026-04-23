#include "CommandQueue.h"
#include "System/Managers/DirectX12Manager/DirectX12Manager.h"

namespace System {

	CommandQueue::CommandQueue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
	{
		if (!device) return;

		D3D12_COMMAND_QUEUE_DESC desc = {};

		//コマンドリストにも種類がある。代表的なのは、
		// D3D12_COMMAND_LIST_TYPE_DIRECT：グラフィックスコマンドリスト。描画やコンピュートのコマンドを記録するためのコマンドリスト
		// D3D12_COMMAND_LIST_TYPE_COPY：コピーコマンドリスト。リソースのコピーや転送のコマンドを記録するためのコマンドリスト
		// D3D12_COMMAND_LIST_TYPE_COMPUTE：コンピュートコマンドリスト。コンピュートシェーダーのコマンドを記録するためのコマンドリスト
		// コマンドキューも、コマンドリストの種類に合わせて、同じ種類のコマンドキューを作成する必要がある

		desc.Type = type;				//指定されたコマンドリストの種類に合わせて、コマンドキューの種類を設定
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	// コマンドキューの優先度を「通常」に設定
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;	// コマンドキューのフラグは特に設定しない
		desc.NodeMask = 0;							// ノードマスクは特に設定しない

		HRESULT hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(command_queue.GetAddressOf()));
		if (hr != S_OK) {
			// コマンドキューの作成に失敗した場合は、失敗を示すためnullのままにして関数を終了する
			return;
		}

		hr = device->CreateFence(fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));
		if (hr != S_OK) {
			// フェンスの作成に失敗した場合は、失敗を示すためキューをリセットして関数を終了する
			command_queue.Reset();
		}
		fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	}

	CommandQueue::~CommandQueue()
	{
		command_queue.Reset();
		fence.Reset();
	}

	int CommandQueue::Execute(const std::vector<ID3D12CommandList*>& command_lists)
	{
		if (!IsValid()) return -1;
		command_queue->ExecuteCommandLists(static_cast<UINT>(command_lists.size()), command_lists.data());

		if (!fence) return -1;
		command_queue->Signal(fence.Get(), ++fence_value);

		return 0;
	}
	int CommandQueue::Execute(const std::vector<ID3D12DeviceContext*>& contexts)
	{
		if (!IsValid()) return -1;
		std::vector<ID3D12CommandList*> command_lists;
		command_lists.reserve(contexts.size());
		for (auto context : contexts) {
			if (context) {
				command_lists.push_back(context->GetCommandList());
			}
		}
		command_queue->ExecuteCommandLists(static_cast<UINT>(command_lists.size()), command_lists.data());

		if (!fence) return -1;
		command_queue->Signal(fence.Get(), ++fence_value);
		for (auto context : contexts) {
			if (context) {
				context->SignalFence(fence_value);
			}
		}
		return 0;
	}


#define IVENT_WAIT
	int CommandQueue::WaitForCompletion(ID3D12DeviceContext* context)
	{


#ifndef IVENT_WAIT


		while (fence->GetCompletedValue() < fence_value)
		{
			// フェンスの完了値がfence_valueに達するまで待機する
			//将来的にはビジーループからイベント駆動方式に変更しなければ
		}
#else
		if (!context) return -1;
		//特定のコンテキストが最後にシグナルしたフェンス値が完了するのを待つ
		if (fence->GetCompletedValue() < context->GetLastSignaledFenceValue())
		{
			// フェンスの完了値がfence_valueに達していない場合は、イベントを待機する
			auto hr = fence->SetEventOnCompletion(context->GetLastSignaledFenceValue(), fence_event);
			if (FAILED(hr))
			{
				// イベントの設定に失敗した場合は、失敗を示す -1 を返す
				return -1;
			}
			WaitForSingleObject(fence_event, INFINITE);
		}


#endif // !IVENT_WAIT
		return 0;
	}
	//自身が最後にシグナルしたフェンス値が完了するのを待つ関数。これを使用することで、コマンドキューに積まれたすべてのコマンドの実行が完了するのを待つことができるようになる。
	int CommandQueue::WaitForCompletionAll()
	{
		size_t completed_value = fence->GetCompletedValue();
		if (completed_value < fence_value)
		{
			// フェンスの完了値がfence_valueに達していない場合は、イベントを待機する
			auto hr = fence->SetEventOnCompletion(fence_value, fence_event);
			if (FAILED(hr))
			{
				// イベントの設定に失敗した場合は、失敗を示す -1 を返す
				return -1;
			}
			WaitForSingleObject(fence_event, INFINITE);
		}

		return 0;
	}
}
