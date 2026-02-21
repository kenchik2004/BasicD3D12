#pragma once

namespace System {
	//D3D12には、デバイスコンテキストが存在しないため、DirectX11のようなデバイスコンテキストを表すクラスは必要ない。
	//ただ、コマンドアロケーターやコマンドリストを管理する必要はあるため、自作でD3D12用のコンテキストを作ることにする。
	class DirectX12Manager;

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
		ComPtr<ID3D12CommandAllocator> command_allocator;
		ComPtr<ID3D12GraphicsCommandList> command_list;
	public:
		ID3D12DeviceContext(ID3D12Device* master_device, D3D12_COMMAND_LIST_TYPE context_type);
		ID3D12CommandAllocator* GetCommandAllocator() const { return command_allocator.Get(); }
		ID3D12GraphicsCommandList* GetCommandList() const { return command_list.Get(); }
		bool IsValid() const { return command_allocator && command_list; }


	};
}

