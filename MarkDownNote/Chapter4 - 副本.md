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

全屏模式，枚举显示模式用于设置成完全一致的，获取最佳性能

## 功能支持监测

ID3D12Device: CheckFeatureSupport









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