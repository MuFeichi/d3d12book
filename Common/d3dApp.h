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
    
	HINSTANCE AppInst()const; //��ȡ���������ڷ���Ӧ�ó���ʵ��
	HWND      MainWnd()const; //��ȡ���������ڷ��������ھ��
	float     AspectRatio()const; // ��̨�������Ŀ�߱�

    bool Get4xMsaaState()const;  //�Ƿ�ʹ��MSAA
    void Set4xMsaaState(bool value); // ����MSAA����ع���

	int Run(); //Run��װ����Ӧ�ó������Ϣѭ��, ����windows��Ϣ or ִ����Ⱦ����
 
    virtual bool Initialize(); // ��ܷ�������ʼ����window��d3d 
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); // win32��Ϣ����

protected:
    virtual void CreateRtvAndDsvDescriptorHeaps(); // ������������
	virtual void OnResize();  // ��קwindow���ڴ�С���������ţ������D3D�������
	virtual void Update(const GameTimer& gt)=0; // ����D3D���������
    virtual void Draw(const GameTimer& gt)=0; // ���ڻ�����ص�Ԫ�أ�

	// Convenience overrides for handling mouse input.
	virtual void OnMouseDown(WPARAM btnState, int x, int y){ }
	virtual void OnMouseUp(WPARAM btnState, int x, int y)  { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y){ }

protected:

	bool InitMainWindow(); // ��ʼ��Ӧ�ó��򴰿�
	bool InitDirect3D(); // ��ʼ��D3D3
	void CreateCommandObjects(); // ����queue��listallocator��list
    void CreateSwapChain(); // ����������

	void FlushCommandQueue(); //ǿ��CPU�ȴ�GPU��֪��GPU������queue�е�����

	ID3D12Resource* CurrentBackBuffer()const; // ���� ������ ��̨���������� RT
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const; // ���ص�ǰ��������Դ��������
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const; // ����DS������

	void CalculateFrameStats(); // ���� FPS����֡��ʱ

    void LogAdapters(); // ö��ϵͳ�е�����������
    void LogAdapterOutputs(IDXGIAdapter* adapter); // ö��ָ����������ȫ����ʾ���
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format); // ö��ĳ����ʾ�������ض���ʽ֧�ֵ���ʾģʽ

protected:

    static D3DApp* mApp; // Class��ָ��

    HINSTANCE mhAppInst = nullptr; // Ӧ�ó���ʵ��application instance handle
    HWND      mhMainWnd = nullptr; // �����ھ�� main window handle
	bool      mAppPaused = false;  // Appֹͣ״̬is the application paused?
	bool      mMinimized = false;  // �Ƿ���С��is the application minimized?
	bool      mMaximized = false;  // �Ƿ����is the application maximized?
	bool      mResizing = false;   // �Ƿ�������קare the resize bars being dragged?
    bool      mFullscreenState = false;// �Ƿ�ȫ���� fullscreen enabled

	// Set true to use 4X MSAA (?.1.8).  The default is false.
    bool      m4xMsaaState = false;    // 4X MSAA enabled
    UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

	// Used to keep track of the
	GameTimer mTimer;
	
    // ģ��
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
    // �û��ڸ���������������캯���ж�����Щ��ʼֵ
	std::wstring mMainWndCaption = L"d3d App";
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	int mClientWidth = 800;
	int mClientHeight = 600;
};

