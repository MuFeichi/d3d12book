//***************************************************************************************
// d3dApp.h by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "d3dUtil.h"
#include "GameTimer.h"

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class D3DApp
{
protected:

    D3DApp(HINSTANCE hInstance);
    D3DApp(const D3DApp& rhs) = delete;
    D3DApp& operator=(const D3DApp& rhs) = delete;
    virtual ~D3DApp();

public:

    static D3DApp* GetApp();
    
	HINSTANCE AppInst()const; //存取函数，用于返回应用程序实例
	HWND      MainWnd()const; //存取函数，用于返回主窗口句柄
	float     AspectRatio()const; // 后台缓冲区的宽高比

    bool Get4xMsaaState()const;  //是否使用MSAA
    void Set4xMsaaState(bool value); // 设置MSAA的相关功能

	int Run(); //Run封装的是应用程序的消息循环, 处理windows消息 or 执行渲染任务
 
    virtual bool Initialize(); // 框架方法。初始化，window和d3d 
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); // win32消息处理

protected:
    virtual void CreateRtvAndDsvDescriptorHeaps(); // 创建描述符堆
	virtual void OnResize();  // 拖拽window窗口大小，进行缩放，会调整D3D相关属性
	virtual void Update(const GameTimer& gt)=0; // 更新D3D程序的设置
    virtual void Draw(const GameTimer& gt)=0; // 用于绘制相关的元素，

	// Convenience overrides for handling mouse input.
	virtual void OnMouseDown(WPARAM btnState, int x, int y){ }
	virtual void OnMouseUp(WPARAM btnState, int x, int y)  { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y){ }

protected:

	bool InitMainWindow(); // 初始化应用程序窗口
	bool InitDirect3D(); // 初始化D3D3
	void CreateCommandObjects(); // 创建queue，listallocator，list
    void CreateSwapChain(); // 创建交换链

	void FlushCommandQueue(); //强制CPU等待GPU，知道GPU处理完queue中的命令

	ID3D12Resource* CurrentBackBuffer()const; // 返回 交换链 后台换缓冲区的 RT
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const; // 返回当前缓冲区资源的描述符
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const; // 返回DS描述符

	void CalculateFrameStats(); // 计算 FPS，和帧耗时

    void LogAdapters(); // 枚举系统中的所有适配器
    void LogAdapterOutputs(IDXGIAdapter* adapter); // 枚举指定适配器的全部显示输出
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format); // 枚举某个显示器对于特定格式支持的显示模式

protected:

    static D3DApp* mApp; // Class的指针

    HINSTANCE mhAppInst = nullptr; // 应用程序实例application instance handle
    HWND      mhMainWnd = nullptr; // 主窗口句柄 main window handle
	bool      mAppPaused = false;  // App停止状态is the application paused?
	bool      mMinimized = false;  // 是否最小化is the application minimized?
	bool      mMaximized = false;  // 是否最大化is the application maximized?
	bool      mResizing = false;   // 是否缩放拖拽are the resize bars being dragged?
    bool      mFullscreenState = false;// 是否全屏化 fullscreen enabled

	// Set true to use 4X MSAA (?.1.8).  The default is false.
    bool      m4xMsaaState = false;    // 4X MSAA enabled
    UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

	// Used to keep track of the
	GameTimer mTimer;
	
    // 模板
    template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

    ComPtr<IDXGIFactory4> mdxgiFactory;
    ComPtr<IDXGISwapChain> mSwapChain;
    ComPtr<ID3D12Device> md3dDevice;

    ComPtr<ID3D12Fence> mFence;
    UINT64 mCurrentFence = 0;
	
    ComPtr<ID3D12CommandQueue> mCommandQueue;
    ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
    ComPtr<ID3D12GraphicsCommandList> mCommandList;

	static const int SwapChainBufferCount = 2;
	int mCurrBackBuffer = 0;
    ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
    ComPtr<ID3D12Resource> mDepthStencilBuffer;

    ComPtr<ID3D12DescriptorHeap> mRtvHeap;
    ComPtr<ID3D12DescriptorHeap> mDsvHeap;

    D3D12_VIEWPORT mScreenViewport; 
    D3D12_RECT mScissorRect;

	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvSrvUavDescriptorSize = 0;

	// Derived class should set these in derived constructor to customize starting values.
    // 用户在改派生类的派生构造函数中定义这些初始值
	std::wstring mMainWndCaption = L"d3d App";
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	int mClientWidth = 800;
	int mClientHeight = 600;
};

