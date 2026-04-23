#pragma once
namespace System {
	//D3D12には、デバイスコンテキストが存在しないため、DirectX11のようなデバイスコンテキストを表すクラスは必要ない。
	//ただ、コマンドアロケーターやコマンドリストを管理する必要はあるため、自作でD3D12用のコンテキストを作ることにする。
	class DirectX12Manager;
	class DescriptorHeap;
	class Texture;
	class ConstantBuffer;

	//-------------------------------------------------------------
	// @brief D3D12用のデバイスコンテキスト
	// @brief コマンドアロケーターやコマンドリストの管理を行うクラス
	// @details D3D12には、デバイスコンテキストが存在しないため、DirectX11のようなデバイスコンテキストを表すクラスは必要ない。
	//			ただ、コマンドアロケーターやコマンドリストを管理する必要はあるので、自作でD3D12用のコンテキストを作ることにする。
	//-------------------------------------------------------------
	class ID3D12DeviceContext
	{
	public:
		virtual ~ID3D12DeviceContext() {
			command_allocator.Reset();
			command_list.Reset();
		};
		// コマンドアロケーターやコマンドリストを管理する関数をここに追加していく
	private:
		bool is_closed = false;	// コマンドリストが閉じているかどうかを管理する変数。コマンドリストが閉じているときは、コマンドの記録や実行ができないようにするために使用する。
		ComPtr<ID3D12CommandAllocator> command_allocator;
		ComPtr<ID3D12GraphicsCommandList> command_list;
		size_t last_signaled_fence_value = 0;	// コマンドリストに記録された最後のコマンドが完了したときのフェンス値を記録する変数。これを管理することで、コマンドリストの実行が完了したかどうかを確認することができるようになる。
	public:
		ID3D12DeviceContext(ID3D12Device* master_device, D3D12_COMMAND_LIST_TYPE context_type);
		ID3D12CommandAllocator* GetCommandAllocator() const { return command_allocator.Get(); }
		ID3D12GraphicsCommandList* GetCommandList() const { return command_list.Get(); }
		int SignalFence(size_t fence_value) {
			last_signaled_fence_value = fence_value;	// 引数で渡されたフェンス値を、最後にシグナルしたフェンス値として記録する
			return 0;
		}
		size_t GetLastSignaledFenceValue() const { return last_signaled_fence_value; }	// 最後にシグナルしたフェンス値を取得する関数。これを使用して、コマンドリストの実行が完了したかどうかを確認することができるようになる。
		bool IsValid() const { return command_allocator && command_list; }
		int CloseCommandList() {
			if (!command_list || is_closed) return -1;	// コマンドリストが有効でない場合は、-1を返す
			is_closed = true; // コマンドリストが閉じられたことを記録する
			return command_list->Close();	// コマンドリストを閉じて、その結果を返す
		}
		int ResetCommandList() {
			if (!command_list || !command_allocator || !is_closed) return -1;	// コマンドリストやコマンドアロケーターが有効でない場合は、-1を返す
			HRESULT hr = command_allocator->Reset();	// コマンドアロケーターをリセットする
			if (FAILED(hr)) return hr;	// リセットに失敗した場合は、その結果を返す
			is_closed = false;	// コマンドリストが開いていることを記録する
			return command_list->Reset(command_allocator.Get(), nullptr);	// コマンドリストをリセットして、その結果を返す
		}
		int SetRenderTarget(Texture* render_target, Texture* dsv);
		int SetRenderTargets(std::array<Texture*, 8> render_targets, Texture* dsv);
		int SetTextureBindless(Texture* texture, unsigned int slot);
		int SetConstantBuffer(ConstantBuffer* cb, unsigned int slot);



	};
}