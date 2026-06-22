// CDirect3D9.cpp — Proxy implementation for IDirect3D9

#include "CDirect3D9.h"
#include "CDirect3DDevice9.h"

CDirect3D9::CDirect3D9(IDirect3D9* pOriginal)
    : m_pD3D(pOriginal)
    , m_ref(1)
{
}

CDirect3D9::~CDirect3D9()
{
    if (m_pD3D) {
        m_pD3D->Release();
        m_pD3D = nullptr;
    }
}

// IUnknown

HRESULT STDMETHODCALLTYPE CDirect3D9::QueryInterface(REFIID riid, void** ppvObj)
{
    return m_pD3D->QueryInterface(riid, ppvObj);
}

ULONG STDMETHODCALLTYPE CDirect3D9::AddRef()
{
    return InterlockedIncrement(&m_ref);
}

ULONG STDMETHODCALLTYPE CDirect3D9::Release()
{
    ULONG ref = InterlockedDecrement(&m_ref);
    if (ref == 0) {
        delete this;
        return 0;
    }
    return ref;
}

// IDirect3D9 — all forwarded to m_pD3D

HRESULT STDMETHODCALLTYPE CDirect3D9::RegisterSoftwareDevice(void* pInitializeFunction)
{
    return m_pD3D->RegisterSoftwareDevice(pInitializeFunction);
}

UINT STDMETHODCALLTYPE CDirect3D9::GetAdapterCount()
{
    return m_pD3D->GetAdapterCount();
}

HRESULT STDMETHODCALLTYPE CDirect3D9::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier)
{
    return m_pD3D->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT STDMETHODCALLTYPE CDirect3D9::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format)
{
    return m_pD3D->GetAdapterModeCount(Adapter, Format);
}

HRESULT STDMETHODCALLTYPE CDirect3D9::EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
    return m_pD3D->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

HRESULT STDMETHODCALLTYPE CDirect3D9::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode)
{
    return m_pD3D->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT STDMETHODCALLTYPE CDirect3D9::CheckDeviceType(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed)
{
    return m_pD3D->CheckDeviceType(Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed);
}

HRESULT STDMETHODCALLTYPE CDirect3D9::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
    return m_pD3D->CheckDeviceFormat(Adapter, DevType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT STDMETHODCALLTYPE CDirect3D9::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels)
{
    return m_pD3D->CheckDeviceMultiSampleType(Adapter, DevType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT STDMETHODCALLTYPE CDirect3D9::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
    return m_pD3D->CheckDepthStencilMatch(Adapter, DevType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT STDMETHODCALLTYPE CDirect3D9::CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat)
{
    return m_pD3D->CheckDeviceFormatConversion(Adapter, DevType, SourceFormat, TargetFormat);
}

HRESULT STDMETHODCALLTYPE CDirect3D9::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps)
{
    return m_pD3D->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HMONITOR STDMETHODCALLTYPE CDirect3D9::GetAdapterMonitor(UINT Adapter)
{
    return m_pD3D->GetAdapterMonitor(Adapter);
}

HRESULT STDMETHODCALLTYPE CDirect3D9::CreateDevice(
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    HWND hFocusWindow,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DDevice9** ppReturnedDeviceInterface)
{
    HRESULT hr = m_pD3D->CreateDevice(Adapter, DeviceType, hFocusWindow,
                                       BehaviorFlags, pPresentationParameters,
                                       ppReturnedDeviceInterface);
    if (SUCCEEDED(hr) && ppReturnedDeviceInterface && *ppReturnedDeviceInterface) {
        // Wrap the real device with our proxy
        CDirect3DDevice9* pProxyDevice = new CDirect3DDevice9(*ppReturnedDeviceInterface);
        *ppReturnedDeviceInterface = pProxyDevice;
    }
    return hr;
}
