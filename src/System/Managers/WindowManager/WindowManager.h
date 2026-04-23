#pragma once
#include "System/SystemUtils/Descriptors/BaseView/View.h"
namespace System {
	class Texture;

	//-------------------------------------------------------------
	// @brief ウィンドウマネージャー
	// @brief ウィンドウとスワップチェインの管理を行うクラス
	//-------------------------------------------------------------
	class WindowManager
	{
	private:
		WindowManager() = default;

		HWND window_handle;
		std::wstring window_class_name;
		std::wstring window_name;

		unsigned int window_width;
		unsigned int window_height;

		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc;
		ComPtr<IDXGISwapChain4> swap_chain;
		int CreateMainWindow();
		// バックバッファのリソースとRTVを管理するための配列。スワップチェインのバッファ数は2に固定しているため、要素数も2にしている。

		static constexpr unsigned int BACK_BUFFER_COUNT = 3;
		std::array<std::unique_ptr<Texture>, BACK_BUFFER_COUNT> back_buffers;

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static inline std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> CustomWindowProc = nullptr;		// カスタムウィンドウプロシージャを設定するための関数オブジェクト。これを設定すると、デフォルトのウィンドウプロシージャの代わりに、カスタムウィンドウプロシージャが呼び出されるようになる。
	public:
		static WindowManager* Instance();

		//-------------------------------------------------------------
		// @brief ウィンドウの情報を設定する関数
		//
		// @param [in] class_name ウィンドウクラス名
		// @param [in] window_name ウィンドウタイトルバーに表示される名前
		// @param [in] width ウィンドウの幅（ピクセル単位）
		// @param [in] height ウィンドウの高さ（ピクセル単位）
		// @warning ウィンドウの情報は、ウィンドウの作成前に設定する必要がある。ウィンドウの作成後にこの関数を呼び出しても、ウィンドウの情報は変更されない。
		//-------------------------------------------------------------
		void SetWindowInfo(const std::wstring& class_name, const std::wstring& window_name, unsigned int width, unsigned int height) {
			if (window_handle)
				return;		// ウィンドウがすでに作成されている場合は、ウィンドウの情報を変更しない
			this->window_class_name = class_name;
			this->window_name = window_name;
			this->window_width = width;
			this->window_height = height;
		}
		void SetCustomWindowProc(std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>&& custom_proc) {
			CustomWindowProc = std::move(custom_proc);
		}
		unsigned int GetWindowWidth() const { return window_width; }
		unsigned int GetWindowHeight() const { return window_height; }
		unsigned int GetBackBufferWidth() const {
			return swap_chain_desc.Width;	// バックバッファの幅を返す	
		}
		unsigned int GetBackBufferHeight() const {
			return swap_chain_desc.Height;	// バックバッファの高さを返す
		}
		HRESULT ResizeBackBuffers(unsigned int width, unsigned int height);

		HWND GetWindowHandle() const { return window_handle; }

		IDXGISwapChain4* GetSwapChain() const { return swap_chain.Get(); }
		Texture* GetCurrentBackBuffer() const {
			UINT back_buffer_index = swap_chain->GetCurrentBackBufferIndex();		// 現在のバックバッファのインデックスを取得
			return back_buffers[back_buffer_index].get();
		}

		//--------------------------------------------------------------
		// @brief スワップチェインを作成する関数
		//--------------------------------------------------------------
		int CreateSwapChain();
		int CreateBackBuffers();
		int ReleaseSwapChain() {
			swap_chain.Reset();
			return 0;
		}

		int Initialize();
		int Finalize();

		int ScreenFlip(unsigned int sync_interval = 0) {
			if (!swap_chain)
				return -1;		// スワップチェインが存在しない場合は、失敗を示す -1 を返す
			HRESULT hr = swap_chain->Present(sync_interval, 0);		// 垂直同期なしで画面を更新する
			return SUCCEEDED(hr);
		}
	};
}