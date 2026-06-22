// CDirect3DDevice9.cpp — Proxy implementation for IDirect3DDevice9
//
// All methods delegate to the real device except:
//   EndScene() — calls renderer::on_end_scene() for custom drawing
//   Reset()    — calls renderer::on_device_lost()/on_device_reset() around the real Reset

#include "CDirect3DDevice9.h"
#include "renderer.h"

CDirect3DDevice9::CDirect3DDevice9(IDirect3DDevice9* pOriginal)
    : m_pDevice(pOriginal)
    , m_ref(1)
{
    d3d9_renderer::on_device_created(m_pDevice);
}

CDirect3DDevice9::~CDirect3DDevice9()
{
    d3d9_renderer::on_device_lost();
    if (m_pDevice) {
        m_pDevice->Release();
        m_pDevice = nullptr;
    }
}

// IUnknown

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::QueryInterface(REFIID riid, void** ppvObj)
{
    return m_pDevice->QueryInterface(riid, ppvObj);
}

ULONG STDMETHODCALLTYPE CDirect3DDevice9::AddRef()
{
    return InterlockedIncrement(&m_ref);
}

ULONG STDMETHODCALLTYPE CDirect3DDevice9::Release()
{
    ULONG ref = InterlockedDecrement(&m_ref);
    if (ref == 0) {
        delete this;
        return 0;
    }
    return ref;
}

// IDirect3DDevice9 — forwarded methods

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::TestCooperativeLevel()
{
    return m_pDevice->TestCooperativeLevel();
}

UINT STDMETHODCALLTYPE CDirect3DDevice9::GetAvailableTextureMem()
{
    return m_pDevice->GetAvailableTextureMem();
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::EvictManagedResources()
{
    return m_pDevice->EvictManagedResources();
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetDirect3D(IDirect3D9** ppD3D9)
{
    return m_pDevice->GetDirect3D(ppD3D9);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetDeviceCaps(D3DCAPS9* pCaps)
{
    return m_pDevice->GetDeviceCaps(pCaps);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode)
{
    return m_pDevice->GetDisplayMode(iSwapChain, pMode);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* pParameters)
{
    return m_pDevice->GetCreationParameters(pParameters);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap)
{
    return m_pDevice->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

void STDMETHODCALLTYPE CDirect3DDevice9::SetCursorPosition(int X, int Y, DWORD Flags)
{
    m_pDevice->SetCursorPosition(X, Y, Flags);
}

BOOL STDMETHODCALLTYPE CDirect3DDevice9::ShowCursor(BOOL bShow)
{
    return m_pDevice->ShowCursor(bShow);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain)
{
    return m_pDevice->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain)
{
    return m_pDevice->GetSwapChain(iSwapChain, pSwapChain);
}

UINT STDMETHODCALLTYPE CDirect3DDevice9::GetNumberOfSwapChains()
{
    return m_pDevice->GetNumberOfSwapChains();
}

// Hooked: Reset()
HRESULT STDMETHODCALLTYPE CDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    d3d9_renderer::on_device_lost();
    HRESULT hr = m_pDevice->Reset(pPresentationParameters);
    if (SUCCEEDED(hr)) {
        d3d9_renderer::on_device_reset();
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::Present(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion)
{
    return m_pDevice->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
    return m_pDevice->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
{
    return m_pDevice->GetRasterStatus(iSwapChain, pRasterStatus);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs)
{
    return m_pDevice->SetDialogBoxMode(bEnableDialogs);
}

void STDMETHODCALLTYPE CDirect3DDevice9::SetGammaRamp(UINT iSwapChain, DWORD Flags, const D3DGAMMARAMP* pRamp)
{
    m_pDevice->SetGammaRamp(iSwapChain, Flags, pRamp);
}

void STDMETHODCALLTYPE CDirect3DDevice9::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp)
{
    m_pDevice->GetGammaRamp(iSwapChain, pRamp);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{
    return m_pDevice->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
{
    return m_pDevice->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{
    return m_pDevice->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{
    return m_pDevice->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{
    return m_pDevice->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
    return m_pDevice->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
    return m_pDevice->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::UpdateSurface(IDirect3DSurface9* pSourceSurface, const RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, const POINT* pDestPoint)
{
    return m_pDevice->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture)
{
    return m_pDevice->UpdateTexture(pSourceTexture, pDestinationTexture);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{
    return m_pDevice->GetRenderTargetData(pRenderTarget, pDestSurface);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface)
{
    return m_pDevice->GetFrontBufferData(iSwapChain, pDestSurface);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::StretchRect(IDirect3DSurface9* pSourceSurface, const RECT* pSourceRect, IDirect3DSurface9* pDestSurface, const RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
    return m_pDevice->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::ColorFill(IDirect3DSurface9* pSurface, const RECT* pRect, D3DCOLOR Color)
{
    return m_pDevice->ColorFill(pSurface, pRect, Color);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
    return m_pDevice->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
    return m_pDevice->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{
    return m_pDevice->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
    return m_pDevice->SetDepthStencilSurface(pNewZStencil);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{
    return m_pDevice->GetDepthStencilSurface(ppZStencilSurface);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::BeginScene()
{
    return m_pDevice->BeginScene();
}

// Hooked: EndScene()
HRESULT STDMETHODCALLTYPE CDirect3DDevice9::EndScene()
{
    d3d9_renderer::on_end_scene(m_pDevice);
    return m_pDevice->EndScene();
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::Clear(DWORD Count, const D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
    return m_pDevice->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX* pMatrix)
{
    return m_pDevice->SetTransform(State, pMatrix);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix)
{
    return m_pDevice->GetTransform(State, pMatrix);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX* pMatrix)
{
    return m_pDevice->MultiplyTransform(State, pMatrix);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetViewport(const D3DVIEWPORT9* pViewport)
{
    return m_pDevice->SetViewport(pViewport);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetViewport(D3DVIEWPORT9* pViewport)
{
    return m_pDevice->GetViewport(pViewport);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetMaterial(const D3DMATERIAL9* pMaterial)
{
    return m_pDevice->SetMaterial(pMaterial);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetMaterial(D3DMATERIAL9* pMaterial)
{
    return m_pDevice->GetMaterial(pMaterial);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetLight(DWORD Index, const D3DLIGHT9* pLight)
{
    return m_pDevice->SetLight(Index, pLight);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetLight(DWORD Index, D3DLIGHT9* pLight)
{
    return m_pDevice->GetLight(Index, pLight);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::LightEnable(DWORD Index, BOOL Enable)
{
    return m_pDevice->LightEnable(Index, Enable);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetLightEnable(DWORD Index, BOOL* pEnable)
{
    return m_pDevice->GetLightEnable(Index, pEnable);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetClipPlane(DWORD Index, const float* pPlane)
{
    return m_pDevice->SetClipPlane(Index, pPlane);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetClipPlane(DWORD Index, float* pPlane)
{
    return m_pDevice->GetClipPlane(Index, pPlane);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
    return m_pDevice->SetRenderState(State, Value);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue)
{
    return m_pDevice->GetRenderState(State, pValue);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
{
    return m_pDevice->CreateStateBlock(Type, ppSB);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::BeginStateBlock()
{
    return m_pDevice->BeginStateBlock();
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::EndStateBlock(IDirect3DStateBlock9** ppSB)
{
    return m_pDevice->EndStateBlock(ppSB);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetClipStatus(const D3DCLIPSTATUS9* pClipStatus)
{
    return m_pDevice->SetClipStatus(pClipStatus);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9* pClipStatus)
{
    return m_pDevice->GetClipStatus(pClipStatus);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture)
{
    return m_pDevice->GetTexture(Stage, ppTexture);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture)
{
    return m_pDevice->SetTexture(Stage, pTexture);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue)
{
    return m_pDevice->GetTextureStageState(Stage, Type, pValue);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
    return m_pDevice->SetTextureStageState(Stage, Type, Value);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
{
    return m_pDevice->GetSamplerState(Sampler, Type, pValue);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
    return m_pDevice->SetSamplerState(Sampler, Type, Value);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::ValidateDevice(DWORD* pNumPasses)
{
    return m_pDevice->ValidateDevice(pNumPasses);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber, const PALETTEENTRY* pEntries)
{
    return m_pDevice->SetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries)
{
    return m_pDevice->GetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber)
{
    return m_pDevice->SetCurrentTexturePalette(PaletteNumber);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetCurrentTexturePalette(UINT* PaletteNumber)
{
    return m_pDevice->GetCurrentTexturePalette(PaletteNumber);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetScissorRect(const RECT* pRect)
{
    return m_pDevice->SetScissorRect(pRect);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetScissorRect(RECT* pRect)
{
    return m_pDevice->GetScissorRect(pRect);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetSoftwareVertexProcessing(BOOL bSoftware)
{
    return m_pDevice->SetSoftwareVertexProcessing(bSoftware);
}

BOOL STDMETHODCALLTYPE CDirect3DDevice9::GetSoftwareVertexProcessing()
{
    return m_pDevice->GetSoftwareVertexProcessing();
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetNPatchMode(float nSegments)
{
    return m_pDevice->SetNPatchMode(nSegments);
}

float STDMETHODCALLTYPE CDirect3DDevice9::GetNPatchMode()
{
    return m_pDevice->GetNPatchMode();
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
    return m_pDevice->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
    return m_pDevice->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
    return m_pDevice->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, const void* pIndexData, D3DFORMAT IndexDataFormat, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
    return m_pDevice->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
{
    return m_pDevice->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::CreateVertexDeclaration(const D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
{
    return m_pDevice->CreateVertexDeclaration(pVertexElements, ppDecl);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
    return m_pDevice->SetVertexDeclaration(pDecl);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
    return m_pDevice->GetVertexDeclaration(ppDecl);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetFVF(DWORD FVF)
{
    return m_pDevice->SetFVF(FVF);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetFVF(DWORD* pFVF)
{
    return m_pDevice->GetFVF(pFVF);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::CreateVertexShader(const DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{
    return m_pDevice->CreateVertexShader(pFunction, ppShader);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9* pShader)
{
    return m_pDevice->SetVertexShader(pShader);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
    return m_pDevice->GetVertexShader(ppShader);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetVertexShaderConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount)
{
    return m_pDevice->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
    return m_pDevice->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetVertexShaderConstantI(UINT StartRegister, const int* pConstantData, UINT Vector4iCount)
{
    return m_pDevice->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
    return m_pDevice->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetVertexShaderConstantB(UINT StartRegister, const BOOL* pConstantData, UINT BoolCount)
{
    return m_pDevice->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
    return m_pDevice->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{
    return m_pDevice->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride)
{
    return m_pDevice->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetStreamSourceFreq(UINT StreamNumber, UINT Setting)
{
    return m_pDevice->SetStreamSourceFreq(StreamNumber, Setting);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting)
{
    return m_pDevice->GetStreamSourceFreq(StreamNumber, pSetting);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
    return m_pDevice->SetIndices(pIndexData);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
    return m_pDevice->GetIndices(ppIndexData);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::CreatePixelShader(const DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
    return m_pDevice->CreatePixelShader(pFunction, ppShader);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9* pShader)
{
    return m_pDevice->SetPixelShader(pShader);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
    return m_pDevice->GetPixelShader(ppShader);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetPixelShaderConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount)
{
    return m_pDevice->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
    return m_pDevice->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetPixelShaderConstantI(UINT StartRegister, const int* pConstantData, UINT Vector4iCount)
{
    return m_pDevice->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
    return m_pDevice->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::SetPixelShaderConstantB(UINT StartRegister, const BOOL* pConstantData, UINT BoolCount)
{
    return m_pDevice->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
    return m_pDevice->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::DrawRectPatch(UINT Handle, const float* pNumSegs, const D3DRECTPATCH_INFO* pRectPatchInfo)
{
    return m_pDevice->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::DrawTriPatch(UINT Handle, const float* pNumSegs, const D3DTRIPATCH_INFO* pTriPatchInfo)
{
    return m_pDevice->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::DeletePatch(UINT Handle)
{
    return m_pDevice->DeletePatch(Handle);
}

HRESULT STDMETHODCALLTYPE CDirect3DDevice9::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{
    return m_pDevice->CreateQuery(Type, ppQuery);
}
