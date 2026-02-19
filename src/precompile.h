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


//シングルトン(実態が1つしか存在しないクラス)を簡単に作るためのテンプレートクラス
template <class T>
class Singleton {
public:

    static inline T* Instance() {
        static T instance_;
        return &instance_;
    }

protected:
    Singleton() {}
    virtual ~Singleton() {}

private:
    Singleton(const Singleton& rhs) {}
    void operator=(const Singleton& rhs) {}
};

#define SINGLETON_CLASS(ClassName) public Singleton<ClassName>

