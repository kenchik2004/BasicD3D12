
#include <Windows.h>
#include <string>

HWND window_handle;



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
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
// @param [out] win_handle 作成されたウィンドウのハンドルが格納される。失敗時はNULL
// @param [in] class_name ウィンドウクラス名
// @param [in] window_name ウィンドウタイトルバーに表示される名前
// @param [in] window_w ウィンドウの幅（ピクセル単位）
// @param [in] window_h ウィンドウの高さ（ピクセル単位）
//
// @return int 成功時は0、失敗時は - 1を返す
// 
//-----------------------------------------------------------------------
int CreateMainWindow(HWND& win_handle, const std::wstring& class_name, const std::wstring& window_name, unsigned int window_w, unsigned int window_h) {

	WNDCLASSW window_class = {};			// 空っぽのウィンドウクラスの構造体を作成

	window_class.lpfnWndProc = WindowProc;				// ウィンドウプロシージャ

	//このウィンドウがどのアプリ(.exe)に属するかを識別するための識別子
	window_class.hInstance = GetModuleHandle(nullptr);		// このアプリケーションのハンドルを取得して設定

	//OSがウィンドウを識別するための名前を設定(基本的になんでもいい)
	window_class.lpszClassName = class_name.c_str();

	//ウィンドウクラスをOSに登録
	RegisterClassW(&window_class);

	//ウィンドウそのものを作成
	win_handle = CreateWindowEx(
		0,						//特にスタイル拡張はしないので0
		class_name.c_str(),		//ウィンドウクラス名
		window_name.c_str(),	//ウィンドウタイトル
		WS_OVERLAPPEDWINDOW,	//ウィンドウスタイル(タイトルバーとサイズ変更可能な枠がある一般的なウィンドウ)

		//ウィンドウの位置とサイズを指定。CW_USEDEFAULTはOSに任せるという意味
		//この場合、
		// ウィンドウの位置はOSに任せる(CW_USEDEFAULT)ので、OSがいい感じに配置してくれる
		// ウィンドウの幅と高さは引数で指定された値になる
		CW_USEDEFAULT, CW_USEDEFAULT, window_w, window_h,

		NULL, NULL,			//親ウィンドウ、メニューハンドルは特に必要ないのでNULL

		//このウィンドウが属するアプリケーションのハンドルを指定(GetModuleHandle(nullptr)で取得したもの)
		window_class.hInstance,
		NULL			//このウィンドウに関連付ける追加のデータがあればここで渡すことができるが、今回は特にないのでNULL
	);


	if (win_handle == NULL) {		//ウィンドウの作成に失敗した場合はNULLが返るので、失敗を示す -1 を返す
		return -1;
	}

	//ウィンドウを表示する。SW_SHOWは「ウィンドウを表示する」という意味の定数
	ShowWindow(win_handle, SW_SHOW);
	//ウィンドウの内容を更新する。これも特に必要ないが、念のため。
	UpdateWindow(win_handle);

	return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{

	std::wstring class_name = L"BasicD3D12Window";		// ウィンドウクラス名
	std::wstring window_name = L"そんなウィンドウ名で大丈夫か?";	// ウィンドウタイトル
	// メインウィンドウの作成と表示
	if (CreateMainWindow(window_handle, class_name, window_name, 800, 600) != 0)
	{
		// ウィンドウの作成に失敗した場合は、アプリケーションを終了する
		return -1;
	}

	MSG msg;		// Windowsのメッセージを格納する構造体

	while (true) {

		// メッセージが来ているか確認。来ていればmsgに格納して、キューから削除する
		// 英語でPeek = 「覗く」という意味。PeekMessageは「メッセージを覗いてみる」という意味で、メッセージが来ているか確認する関数
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			// 来ていた場合、TranslateMessageでキーボード入力などのメッセージを翻訳して、DispatchMessageでウィンドウプロシージャに送る
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// WM_QUIT(「アプリ終わるでー」のメッセージ)が来ていたら、ループを抜けてアプリを終了する
		if (msg.message == WM_QUIT) {
			break;
		}

		// ここにゲームの更新や描画のコードを入れることになる

	}
	UnregisterClassW(L"BasicD3D12Window", GetModuleHandle(nullptr));		// ウィンドウクラスの登録を解除する
	return 0;
}
