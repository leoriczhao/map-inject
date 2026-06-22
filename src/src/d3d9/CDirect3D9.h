// CDirect3D9.h — Proxy wrapper for IDirect3D9
//
// Intercepts CreateDevice to wrap IDirect3DDevice9 with our CDirect3DDevice9 proxy.
// All other methods are forwarded to the real IDirect3D9.

#pragma once

#include "d3d9_proxy.h"

class CDirect3D9 : public IDirect3D9
{
public:
    CDirect3D9(IDirect3D9* pOriginal);
    virtual ~CDirect3D9();

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override;
    ULONG   STDMETHODCALLTYPE AddRef() override;
    ULONG   STDMETHODCALLTYPE Release() override;

    // IDirect3D9
    HRESULT STDMETHODCALLTYPE RegisterSoftwareDevice(void* pInitializeFunction) override;
    UINT    STDMETHODCALLTYPE GetAdapterCount() override;
    HRESULT STDMETHODCALLTYPE GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier) override;
    UINT    STDMETHODCALLTYPE GetAdapterModeCount(UINT Adapter, D3DFORMAT Format) override;
    HRESULT STDMETHODCALLTYPE EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode) override;
    HRESULT STDMETHODCALLTYPE GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode) override;
    HRESULT STDMETHODCALLTYPE CheckDeviceType(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed) override;
    HRESULT STDMETHODCALLTYPE CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) override;
    HRESULT STDMETHODCALLTYPE CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) override;
    HRESULT STDMETHODCALLTYPE CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) override;
    HRESULT STDMETHODCALLTYPE CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) override;
    HRESULT STDMETHODCALLTYPE GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) override;
    HMONITOR STDMETHODCALLTYPE GetAdapterMonitor(UINT Adapter) override;
    HRESULT STDMETHODCALLTYPE CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) override;

private:
    IDirect3D9* m_pD3D;  // The real IDirect3D9
    LONG        m_ref;   // Reference count
};
