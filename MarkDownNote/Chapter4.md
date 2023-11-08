# Pre 预备知识

## 纹理格式

用于描述纹理的数据格式信息
typeless格式的纹理仅用来预留内存大小，绑定到pipeline之后，具体解释数据类型

## 交换链与页面翻转

页面翻转，就交换前后缓冲区（Front Buffer 和 Back buffer）的过程
![image-20231010212129412](./TextureCache/image-20231010212129412.png)

上述结构，前台缓冲区和后天缓冲区构成的是交换链，一般是双缓冲结构
相关API： 

```C++
IDXGISwapChain //（存放的是前后两张Buffer，纹理Buffer）
IDXGISwapChain::ResizeBuffers
IDXGISwapChain::Present // 前后台互换的过程被称作 Present，呈现 显示
```

## 深度缓冲区

Depth Buffer，or Z buffer，仅对小于深度缓冲区（通过Depth Test）的像素进行绘制
深度缓冲区域也是一种纹理格式，一般有以下几种格式
![image-20231010212117055](./TextureCache/image-20231010212117055.png)

用到 Depth就会用 Stencil，一般都是绑定使用的

## 资源与描述符

每次绘制 DrawCall使用的 资源需要绑定Bind 到 渲染管线上
绑定机制是通过 描述符来实现（descriptor），实现间接引用
可以认为，描述符是一种比较轻量的Struct，用于描述资源的结构，是一个中间层
GPU可以通过描述符 获取实际的资源数据 + 资源的信息，描述符的主要作用是解释资源如何被使用（绑定在那个阶段上）
也可以通过描述符获取局部数据
如果资源创建时，是typeless格式，那么创建描述符要指明具体类型

View（视图）和 descriptor（描述符）是同义词

常见描述符类型

| 简称    | 解释                                    |
| ------- | --------------------------------------- |
| CBV     | 常量缓冲区视图，constant buffer view    |
| SRV     | 着色器资源视图，Shader Resource View    |
| UAV     | 无序访问资源视图，Unordered access view |
| Sampler | 采样器资源资源，用于纹理贴图的          |
| RTV     | 渲染目标视图资源，Render Target View    |
| DSV     | 深度\模板视图资源，Depth Stencil View   |

Descriptor Heap：描述符堆。可以看做是数组，描述符的集合。本质上是存放用户程序中某种特定类型描述符的一块内存
一种类型，创建出一个或者多个描述符堆。

多个描述符--->同一个资源。多个描述符引用同一个资源的不同局部数据
一个资源可以绑定到单挑pipeline的不同阶段，每个阶段都需要设置独立的描述符
比如：纹理可以用作渲染目标和着色器资源视图。需要设置RTV和SRV
无类型格式创建一个资源，可以创建两个描述符，一个指定为浮点，一个指定为整数

## 多重多采样技术

supersampling：SSAA，超采样，4倍与屏幕大小分辨率创建后天缓冲区和深度缓冲区，到屏幕上的时候，周围4个像素一组进行解析（Resolve，降采样DownSample），获得相对平滑的像素颜色

multisampling：多重采样，MSAA，多重采样依旧是会使用4X分辨率，也就是放大进行渲染，然后选取四个像素进行解析，
不同的是会考虑多边形的内外关系和Depth Stencil的可见性，直接复制到可见性像素中，会节省资源

多重采样案例：
DXGI_SAMPLE_DESC
D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS

## 功能分级 Feature Level

D3D_FEATURE_LEVEL对应不同D3D版本之间的各种功能

## DirectX图形基础结构

DXGI = DirectX Graphic Infrastructure，DXGI
底层任务借助一组通用API来进行处理，一般都用DXGI来命名，比如交换链，全屏，适配显示器，显示模式，支持的表面格式信息DXGI_FORMAT

IDXGIFactory，用于创建IDXGISwapChain接口和枚举适配显示器IDXGIAdapter（显卡输出，核心，集显的区别）
显示器or显示设备：Display Output；Adapter output。IDXGIOutput来表示，每个适配器与一组显示输出关联
显示设备都有支持的显示模式：DXGI_MODE_DESC; DXGI_RATIONAL;DXGI_MODE_SCANLING_ORDER;DXGI_MODE_SCALING

ID3D12Device一般指的是某一个显卡适配器，或者说显卡！

全屏模式，枚举显示模式用于设置成完全一致的，获取最佳性能

## 功能支持监测

```c++
HRESULT ID3D12Device::CheckFeatureSupport(
D3D12_FEATURE Feature,
void *pFeatureSupportData,
UINT FeatureSupportDataSize);ID3D12Device: CheckFeatureSupport
```

1. Feature：指定希望检测的功能支持类型

   | Feature类型                              | 检测功能                                                     |
   | ---------------------------------------- | ------------------------------------------------------------ |
   | D3D12_FEATURE_D3D12_OPTIONS              | 检测当前图形驱动对 Direct3D 12 各种功能的支持情况            |
   | D3D12-FEATURE_ARCHITECTURE               | 检测图形适配器中 GPU 的硬件体系架构特性。                    |
   | D3D12_FEATURE_FEATURE_LEVELS             | 检测对功能级别的支持情况。                                   |
   | D3D12_FEATURE_FORMAT_SUPPORT             | 检测对给定纹理格式的支持情况(例如，指定的格式能否用于渲染目标？或，指定的格式能否用于混合技术？）。 |
   | D3D12-FEATURE-MULTISAMPLE-QUALITY-LEVELS | 检测对多重采样功能的支持情况。                               |

2. pEeatureSupportData：指向某种数据结构的指针，该数据结构中存在检索到特定功能支持的信息，具体类型取决于Feature

   | Feature类型                              | Data实例                                      |
   | ---------------------------------------- | --------------------------------------------- |
   | D3D12_FEATURE_D3D12_OPTIONS              | D3D12_FEATURE_DATA_D3D12_OPTIONS 实例         |
   | D3D12-FEATURE_ARCHITECTURE               | D3D12_FEATURE_DATA_ARCHITECTURE               |
   | D3D12_FEATURE_FEATURE_LEVELS             | D3D12_FEATURE_DATA_FEATURE_LEVELS             |
   | D3D12_FEATURE_FORMAT_SUPPORT             | D3D12_FEATURE_DATA_FORMAT_SUPPORT             |
   | D3D12-FEATURE-MULTISAMPLE-QUALITY-LEVELS | D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS |

3. FeatureSupportDataSize：传回上述参数中数据结构的大小



## 资源驻留

D3D12可以主动控制资源在显存中的去留（residency）
无论资源是否以以及位于显存中，都可以进行管理
程序应当避免在短时间内于显存中交换进出相同的资源，清出资源在短时间内不会再使用

```c++
//驻留资源
HRESULT ID3D12Device::MakeResident(
UINT NumObjects,
ID3D12Pageable *const *ppobjects);
//清出资源
HRESULT ID3D12Device::Evict(
UINTNumObjects,
ID3D12Pageable *const *ppobjects);
```

第一个参数表示资源数量，第二个参数表示资源数组



## 关于 Component Object Model 与 ComPtr

Component Object Model：可以理解成为接口集合，当成一个类用去使用
获取指向某个 Component 的指针 一般需要借助特定的函数接口，而不是需要去New一个出来
一般来说，Com Object 会统计下引用的次数，一般用release来减少一次引用
引用为0，自动进行回收，而非使用delete进行现实的删除

接口释放问题：
比较很传统的做法是从 IUnknown类上，使用三个接口进行，一般只用释放，方式一般用宏来进行，而非直接调用release接口

```
// d3dUtil.h
#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif
```

![image-20231010212140804](./TextureCache/image-20231010212140804.png)

智能指针 ComPtr
因为进行手动管理比较麻烦，容易出错，智能指针主要是会对Com对象的release进行自动调用
**该智能指针的大小和一般的指针大小是一致的**，没有额外的内存空间占用
智能指针需要包含头文件`wrl/client.h`，并且智能指针类模板`ComPtr`位于名称空间`Microsoft::WRL`内
COM的接口一般都用大写的'I'来开头，比如

```
ID3D12GraphicsCommandList
```

常用的方法，是123，第四第五不常用

| 方法                           | 描述                                                         |
| ------------------------------ | ------------------------------------------------------------ |
| ComPtr::Get                    | 该方法返回T*，并且不会触发引用计数加1，常用在COM组件接口的函数输入<br />常用于把原始的COM接口指针作为参数传递给函数![image-20231010212210742](./TextureCache/image-20231010212210742.png) |
| ComPtr::GetAddressOf           | 该方法返回指向底层COM接口指针的地址，<br />该方法返回`T**`，常用在COM组件接口的函数输出<br />![image-20231010212223934](./TextureCache/image-20231010212223934.png) |
| ComPtr::Reset                  | 该方法对里面的实例调用Release方法，并将指针置为`nullptr`，直接将Comptr的实例赋值为nullptr相同 |
| ComPtr::ReleaseAndGetAddressOf | 该方法相当于先调用`Reset`方法，再调用`GetAddressOf`方法获取`T**`，常用在COM组件接口的函数输出，适用于实例可能会被反复构造的情况下 |
| ComPtr::As                     | 一个模板函数，可以替代`IUnknown::QueryInterface`的调用，需要传递一个ComPtr实例的地址 |

| &      | 相当于调用了`ComPtr<T>::ReleaseAndGetAddressOf`方法，**不推荐使用** |
| ------ | ------------------------------------------------------------ |
| ->     | 和裸指针的行为一致                                           |
| =      | 不要将裸指针指向的实例赋给它，若传递的是ComPtr的不同实例则发生交换 |
| ==和!= | 可以和`nullptr`，或者另一个ComPtr实例进行比较                |

C++ 可读性 小技巧： 使用 模板的别名增加可读性![image-20231010212235160](./TextureCache/image-20231010212235160.png)

# CPU与GPU的交互

Direct3D 11 支持两种绘制方式:即立即渲染( immediate rendering，利用 immediate context实现)以及延迟渲染(deferred rendering,利用deferred context实现)。
前者将缓冲区中的命令直接借驱动层发往GPU 执行，后者则与本文中介绍的命令列表模型相似(但执行命令列表时仍然要依赖immediate context)。
到了 Direct3D 12 便取消了立即渲染方式,完全采用“命令列表->命令队列”模型，使多个命令列表同时记录命令,借此充分发挥多核心处理器的性能。

## 命令队列与命令列表

GPU维护Command queue，环形缓冲区，ring buffer
CPU维护Command List，将命令提交到Command queue中
从list到queue的命令不会被立即执行，新命令会在队列之中等待执行
![image-20231107114853028](./TextureCache/image-20231107114853028.png)

命令队列：ID3D12CommandQueue
描述：D3D12_COMMAND_QUEUE_DESC
创建API: ID3D12Device::CreatCommandQueue

```c++
Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
D3D12 COMMAND QUEUE_DESC queueDesc= {};
queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
ThrowIfFailed (md3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));
```

![image-20231107115506835](./TextureCache/image-20231107115506835.png)IID_PPV_ARGS作用：获取COM接口ID，将PPType强制转化为Void** 类型

ExecuteCommandLists是ID3D12CommandQueue的接口方法，将List内的命令添加到命令队列中

```c++
void ID3D12CommandQueue::ExecuteCommandLists(
// 第二个参数里命令列表数组中命令列表的数量
UINT Count,
// 待执行的命令列表数组，指向命令列表数组中第一个元素的指针
ID3D12CommandList *const *ppCommandLists);
```

ID3D12GraphicsCommandList 封装了一系列图形渲染命令，实际上继承于ID3D12CommandList。代码示例

```c++
// mCommandList 为一个指向 ID3D12CommandList 接口的指针
mCommandList->RSSetViewports(1,&mScreenViewport); // 添加设置ViewPort
// 清楚RTV
mCommandList->ClearRenderTargetView (mBackBufferView,Colors::LightSteelBlue, 0, nullptr);
// 发起绘制调用命令
mCommandList->DrawIndexedInstanced(36, 1, 0,0, 0);
```

List创建

```c++
HRESULT ID3D12Device::CreateCommandList(
UINT nodeMask,
D3D12_COMMAND_LIST_TYPE type,
ID3D12CommandAllocator *pCommandAllocator,
ID3D12PipelineState *pInitialState,
REFIID riid,
void **ppCommandList);
```

| 参数列表          | 含义                                                         |
| ----------------- | ------------------------------------------------------------ |
| nodeMask          | 仅有一个GPU的系统，设置为0，多CPU系统，nodemask是指List相关的物理GPU<br />ID3D12Device::GetNodeCount，GPU适配器节点数量 |
| type              | List类型                                                     |
| pCommandAllocator | List相关的Allocator，与上述的type需要保持一致                |
| pInitialState     | 指定List的渲染流水管线的初始状态。打包类型可以是nullptr      |
| riid              | 待创建ID3D12CommandList的接口COM ID                          |
| ppCommandlIST     | 输出指向所建List的指针                                       |

添加完成后，需要再调用ExecuteComm的andLists加入Queue
命令都添加完毕后，需要结束命令的记录，在加入队列前，一定要关闭记录！！！！！

```c++
// OrgAPI: ID3D12GraphicsCommandList：：Close
mCommandList->Close ();
```

ID3D12CommandAllocator：List内的命令仅仅是索引，实际存放在CommandAllocator上。queue从list中获取命令时，主要引用的是allocator里面的命令。Allocator是由ID3D12Device创建的

```c++
HRESULT ID3D12Device::CreateCommandAllocator(
D3D12_COMMAND_LIST_TYPE type,
REFIID riid,
void **ppCommandAllocator);
```

| 参数               | 含义                                                         |
| ------------------ | ------------------------------------------------------------ |
| Type               | 命令列表类型，常用两种<br />1. D3D12_COMMAND_LIST_TYPE_DIRECT：GPU直接执行的命令<br />2. D3D12_COMMAND_LIST_TYPE_BUNDLE：命令列表打包bundle。构建List会产生CPU开销<br />D3D12提供优化，将List打包，打包完成(记录完毕)，驱动对命令进行预处理，渲染期间的之间进行优化 |
| riid               | 待创建ID3D12CommandAllocator接口的COM ID                     |
| ppCommandAllocator | 输出指向所建命令分配器的指针                                 |

可以创建多个List关联于同一个Allocator，但是不能同时记录command
当一个list记录时，Allocator中其他的list都是关闭的，保证一个list的命令连续的添加到allocator中
create或者reset list会处于打开的状态，占用报错如下
![image-20231107161041874](./TextureCache/image-20231107161041874.png)
ID3D12GraphicsCommandList::Reset：用于安全的复用list占用的内存来记录新的命令

```c++
HRESULT ID3D12GraphicsCommandList::Reset(
ID3D12CommandAllocator *pAllocator,
ID3D12PipelineState *pInitialState) ;
```

List恢复为创建时的初始状态，可以复用底层的内存，避免释放重新分配的问题。并且不会影响queue中的命令，因为Allocator中存在的是实际的命令

ID3D12CommandAllocator::Reset：是向GPU提交了一帧的渲染命令后，为了下一帧复用Allocator的内存进行重新设置
一般参数是Void，vector::clear，内容清零，但是仍然保存有当前的内存占用。
所以需要在确定GPU执行完Allocator上所有的命令后再进行重置

## CPU与GPU的同步

经典问题保证绘制R的P1，之后绘制R的P2。需要保证不被跳过，并且绘制R的P1时，R的数据不被修改![image-20231108152219164](./TextureCache/image-20231108152219164.png)
强制CPU等待，CPU处理所有命令，达到某个围栏点(Fence point)，刷新命令队列(Flushing the command queue)，
通过围栏来实现，ID3D12Fence。这个操作会导致CPU空闲

```c++
HRESULT ID3D12Device::CreateFence(
	UINT64 InitialValue,
	D3D12_FENCE_FLAGS Flags,
	REFIID riid,
	Void **ppFence);

ThrowIfFailed(md3Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
```

UINT64的值是表示围栏点的整数，初始化为0，每需要增加一个值就加一
ID3D12CommandQueue::Signal从GPU端设置围栏值
ID3D12Fence::Singal 从CPU端设置围栏值

```c++
UINT64 mCurrentFence = 0;
void D3DApp::FlushCommandQueue()
{
    //增加围栏值，接下来将命令标记到此围栏点
	mCurrentFence++;
	// 向命令队列中添加一条用来设置新围栏点的命令
	// 由于这条命令要交由 GPU 处理(即由 GPU 端来修改围栏值),所以在GPU 处理完命令队列中此 Signal ()
	// 的所有命令之前，它并不会设置新的围栏点①
	ThrowIfFailed (mCommandQueue->Signal (mFence.Get (), mCurrentFence));
    
	// 在 CPU 端等待 GPU，直到后者执行完这个围栏点之前的所有命令
	if (mFence->GetCompletedValue() < mCurrentFence)
    {
       	HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        //若GPU命中当前的围栏(即执行到Signal ()指令,修改了围栏值),则激发预定事件
        ThrowIfFailed(mFence->SetEventOnCompletion (mCurrentFence, eventHandle));
        // 等待 GPU 命中围栏，激发事件
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle); 
    }
}
```

![image-20231108174524429](./TextureCache/image-20231108174524429.png)



