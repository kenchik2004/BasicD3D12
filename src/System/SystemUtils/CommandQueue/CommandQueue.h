#pragma once

namespace System {
	class ID3D12DeviceContext;
	//-------------------------------------------------------------
	// @brief コマンドキュークラス
	// @brief コマンドキュー、フェンス、フェンス値を所有・管理するクラス
	//-------------------------------------------------------------
	class CommandQueue
	{
	public:
		CommandQueue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);
		~CommandQueue();

		// @brief 渡されたコマンドリストをキューに積み、実行する
		int Execute(const std::vector<ID3D12CommandList*>& command_lists);
		int Execute(const std::vector<ID3D12DeviceContext*>& contexts);

		// @brief キューの実行完了を待つ
		int WaitForCompletion(ID3D12DeviceContext* context);
		int WaitForCompletionAll();

		ID3D12CommandQueue* GetCommandQueue() const { return command_queue.Get(); }
		bool IsValid() const { return command_queue && fence && fence_event; }

	private:
		ComPtr<ID3D12CommandQueue> command_queue;
		ComPtr<ID3D12Fence> fence;
		size_t fence_value = 0;
		HANDLE fence_event = nullptr;
	};
}
