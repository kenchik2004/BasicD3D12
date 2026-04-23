#pragma once

//MicrosoftのWindows APIを使用するためのヘッダーファイル
#define NOMINMAX //Windows.hのmin,maxマクロを無効化
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

//stdライブラリのヘッダーファイル
#include <vector>
#include <array>
#include <string>
#include <memory>
#include <functional>
#include <map>
#include <unordered_map>
#include <fstream>
#include <filesystem>

#include "DirectXTex.h"

#ifndef NDEBUG
// デバッグビルドの場合、DirectXTexのデバッグバージョンをリンク
#define TEX_DIR "ExternalLibrary/DirectXTex/x64/Debug/DirectXTex.lib"
//assimpもデバッグバージョンをリンク
#define ASSIMP_DIR "ExternalLibrary/assimp/x64/Debug/assimp-vc143-mtd.lib"
#else
// リリースビルドの場合、DirectXTexのリリースバージョンをリンク
#define TEX_DIR "ExternalLibrary/DirectXTex/x64/Release/DirectXTex.lib"
#define ASSIMP_DIR "ExternalLibrary/assimp/x64/Release/assimp-vc143-mt.lib"
#endif // !NDEBUG
#pragma comment(lib, TEX_DIR)
#pragma comment(lib, ASSIMP_DIR)
