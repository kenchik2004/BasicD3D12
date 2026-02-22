#pragma once

//MicrosoftのWindows APIを使用するためのヘッダーファイル
#include <Windows.h>


//DirectX 12を使用するためのヘッダーファイル
#include <d3d12.h>
#include <dxgi1_6.h>

//DirectX12のライブラリのリンク
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

//MicrosoftのWRL(Windows Runtime Library)を使用するためのヘッダーファイル
#include <wrl/client.h>

//DirectX12のオブジェクトを安全に管理するためのスマートポインタのエイリアス(別名)
template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

