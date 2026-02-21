#include "precompile.h"
#include "WindowManager.h"
#include "System/Managers/DirectX12Manager/DirectX12Manager.h"
#include "System/SystemUtils/DeviceContext/ID3D12DeviceContext.h"
#include "System/SystemUtils/Descriptors/BaseView/View.h"

namespace System {


	//ウィンドウプロシージャとは
	// ウィンドウプロシージャは、OSがウィンドウに対して発生するイベントやメッセージを処理するための関数

	//-----------------------------------------------------------------------
	// @brief ウィンドウプロシージャ。ウィンドウに送られてくるメッセージを処理する関数。
	//
	// @param [in] hwnd メッセージが送られてきたウィンドウのハンドル
	// @param [in] uMsg メッセージの種類を示す定数
	// @param [in] wParam メッセージに関連する追加情報
	// @param [in] lParam メッセージに関連する追加情報
	//
	//------------------------------------------------------------------------
	LRESULT CALLBACK WindowManager::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {		// メッセージを確認

		case WM_DESTROY:		// 内容が「ウィンドウが破棄されたよ」なら

			PostQuitMessage(0);			// メッセージループにWM_QUIT(「アプリ終わるでー」のメッセージ)を送る
			return 0;
		}
		// それ以外のメッセージはデフォルトの処理を行う
		return DefWindowProc(hwnd, uMsg, wParam, lParam);		//Windowsのデフォルト処理
	}

	//-----------------------------------------------------------------------
	// @brief メインウィンドウを作成し、表示・初期化を行う関数。
	//
	// @param [in] class_name ウィンドウクラス名
	// @param [in] window_name ウィンドウタイトルバーに表示される名前
	// @param [in] window_w ウィンドウの幅（ピクセル単位）
	// @param [in] window_h ウィンドウの高さ（ピクセル単位）
	//
	// @return int 成功時は0、失敗時は - 1を返す
	// 
	//-----------------------------------------------------------------------
	int WindowManager::CreateMainWindow() {


		WNDCLASSEX window_class = {};			// 空っぽのウィンドウクラスの構造体を作成

		window_class.cbSize = sizeof(WNDCLASSEX);	// ウィンドウクラスの構造体のサイズを設定

		window_class.lpfnWndProc = WindowProc;				// ウィンドウプロシージャ

		//このウィンドウがどのアプリ(.exe)に属するかを識別するための識別子
		window_class.hInstance = GetModuleHandle(NULL);		// このアプリケーションのハンドルを取得して設定

		//OSがウィンドウを識別するための名前を設定(基本的になんでもいい)
		window_class.lpszClassName = window_class_name.c_str();

		//ウィンドウクラスをOSに登録
		RegisterClassEx(&window_class);

		//ウィンドウのクライアント領域(実際に描画できる領域)のサイズを指定するための構造体を作成
		RECT rect = { 0,0,(LONG)window_width,(LONG)window_height };

		//ウィンドウのスタイル(WS_OVERLAPPEDWINDOW)に合わせて、ウィンドウ全体のサイズを調整する。
		//これにより、書き込み領域が指定されたサイズになるようにする
		AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

		//ウィンドウそのものを作成
		window_handle = CreateWindowEx(
			0,						//特にスタイル拡張はしないので0
			window_class_name.c_str(),		//ウィンドウクラス名
			window_name.c_str(),	//ウィンドウタイトル
			WS_OVERLAPPEDWINDOW,	//ウィンドウスタイル(タイトルバーとサイズ変更可能な枠がある一般的なウィンドウ)

			//ウィンドウの位置とサイズを指定。CW_USEDEFAULTはOSに任せるという意味
			//この場合、
			// ウィンドウの位置はOSに任せる(CW_USEDEFAULT)ので、OSがいい感じに配置してくれる
			// ウィンドウの幅と高さは引数で指定された値になる
			CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,

			NULL, NULL,			//親ウィンドウ、メニューハンドルは特に必要ないのでNULL

			//このウィンドウが属するアプリケーションのハンドルを指定(GetModuleHandle(NULL)で取得したもの)
			window_class.hInstance,
			NULL			//このウィンドウに関連付ける追加のデータがあればここで渡すことができるが、今回は特にないのでNULL
		);


		if (window_handle == NULL) {		//ウィンドウの作成に失敗した場合はNULLが返るので、失敗を示す -1 を返す
			return -1;
		}

		//ウィンドウを表示する。SW_SHOWは「ウィンドウを表示する」という意味の定数
		ShowWindow(window_handle, SW_SHOW);
		//ウィンドウの内容を更新する。これも特に必要ないが、念のため。
		UpdateWindow(window_handle);

		return 0;
	}



	int WindowManager::CreateSwapChain()
	{
		DXGI_SWAP_CHAIN_DESC1 desc = {};
		desc.Width = window_width;
		desc.Height = window_height;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;		// バックバッファのフォーマットをRGBA 8ビットに設定
		desc.Stereo = false;						// ステレオ表示は使用しない
		desc.SampleDesc.Count = 1;					// マルチサンプリングは使用しない
		desc.SampleDesc.Quality = 0;				// マルチサンプリングの品質レベルは0
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// バッファの使用方法をレンダーターゲット出力に設定
		desc.BufferCount = 2;						// バッファの数を2に設定（ダブルバッファリング）
		desc.Scaling = DXGI_SCALING_STRETCH;		// スケーリングモードを「引き伸ばし」に設定

		//スワップエフェクト(画面出力を行った後のバッファの扱い)を、出力後即破棄するように設定
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;	// アルファモードは特に指定しない

		//モニターのリフレッシュレートに合わせて自動的にフルスクリーンとウィンドウモードを切り替えることを許可するフラグを設定
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		IDXGIFactory6* factory = DirectX12Manager::Instance()->GetFactory();	// DirectX12のファクトリーを取得
		ID3D12CommandQueue* command_queue = DirectX12Manager::Instance()->GetDrawQueue();	// コマンドキューを取得

		if (!factory || !command_queue) {
			// ファクトリーやコマンドキューの取得に失敗した場合は、失敗を示す -1 を返す
			return -1;
		}

		HRESULT hr = factory->CreateSwapChainForHwnd(
			command_queue,			// スワップチェインを作成するためのコマンドキュー
			window_handle,			// スワップチェインを関連付けるウィンドウのハンドル
			&desc,					// スワップチェインの設定を指定する構造体へのポインタ
			nullptr,				// フルスクリーンモードの設定は特に必要ないのでnullptrを指定
			nullptr,				// 共有リソースの設定は特に必要ないのでnullptrを指定
			(IDXGISwapChain1**)swap_chain.GetAddressOf()	// 作成されたスワップチェインのインターフェースへのポインタを受け取るための引数
		);

		if (hr != S_OK) {		// スワップチェインの作成に失敗した場合は、失敗を示す -1 を返す
			return -1;
		}


		return CreateBackBuffers();		// バックバッファの作成を行う
	}

	int WindowManager::CreateBackBuffers()
	{
		DXGI_SWAP_CHAIN_DESC desc = {};
		swap_chain->GetDesc(&desc);		// スワップチェインの設定を取得して、descに保存する
		//スワップチェインのバックバッファ用RTVを作成する
		for (unsigned int i = 0; i < desc.BufferCount; i++) {		// バッファの数だけ繰り返す
			ComPtr<ID3D12Resource> back_buffer;		// バックバッファのリソースを受け取るための変数
			HRESULT hr = swap_chain->GetBuffer(i, IID_PPV_ARGS(back_buffer.GetAddressOf()));		// スワップチェインのバックバッファを取得

			if (hr != S_OK) {		// バックバッファの取得に失敗した場合は、失敗を示す -1 を返す
				return -1;
			}

			D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};		// RTVの記述を作成
			rtv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtv_desc.Texture2D.MipSlice = 0;	// バックバッファのミップレベルは0（フル解像度）を指定
			rtv_desc.Texture2D.PlaneSlice = 0;	// バックバッファのプレーンは0を指定
			std::unique_ptr<RenderTargetView> rtv =
				DirectX12Manager::Instance()->
				CreateRenderTargetView(back_buffer.Get(), rtv_desc);		// バックバッファのRTVを作成
			if (rtv == nullptr) {		// RTVの作成に失敗した場合は、失敗を示す -1 を返す
				return -1;
			}
			back_buffers[i] = std::move(rtv);		// 作成したRTVをバックバッファのRTV配列に保存する

		}
		return 0;
	}


	int WindowManager::Initialize() {
		if (CreateMainWindow() != 0) {		// メインウィンドウの作成に失敗したら、失敗を示す -1 を返す
			return -1;
		}

		return 0;
	}
	int WindowManager::Finalize() {

		UnregisterClass(window_class_name.c_str(), GetModuleHandle(NULL));		// ウィンドウクラスの登録を解除する
		return 0;
	}
}