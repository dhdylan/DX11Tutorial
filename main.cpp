#include "d3dApp.h"
#include <stdio.h>
#include <iostream>
#include <tchar.h>

class MyD3DApp : public D3DApp
{
public:
	MyD3DApp(HINSTANCE hInstance);
	~MyD3DApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	MyD3DApp theApp(hInstance);
	if (!theApp.Init())
		return 0;
	return theApp.Run();
}

MyD3DApp::MyD3DApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

MyD3DApp::~MyD3DApp()
{
}

bool MyD3DApp::Init()
{
	if (!D3DApp::Init())
		return false;

	//disable alt+enter fullscreen
	IDXGIDevice* dxgiDevice = 0;
	HR(md3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

	IDXGIAdapter* dxgiAdapter = 0;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

	IDXGIFactory* dxgiFactory = 0;
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

	IDXGIAdapter* adapter = 0;

	int i = 0;
	while (dxgiFactory->EnumAdapters(i++, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		LARGE_INTEGER li;
		if (adapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &li) == S_OK)
		{
			OutputDebugStringW(L"\nDevice supported\n");
		}
		else
		{
			OutputDebugStringW(L"\nDevice not supported\n");
		}

		IDXGIOutput* output = 0;

		int j = 0;

		while (adapter->EnumOutputs(j++, &output) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_OUTPUT_DESC desc;
			ZeroMemory(&desc, sizeof(desc));

			output->GetDesc(&desc);
			OutputDebugStringW(L"\n***");
			OutputDebugStringW(desc.DeviceName);
			OutputDebugStringW(L"\n");

			ReleaseCOM(output);
		}

		ReleaseCOM(adapter);
	}

	ReleaseCOM(dxgiFactory);
	ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdapter);


	return true;
}

void MyD3DApp::OnResize()
{
	D3DApp::OnResize();
}

void MyD3DApp::UpdateScene(float dt)
{
}

void MyD3DApp::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);
	// Clear the back buffer blue. Colors::Blue is defined in d3dUtil.h.
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView,
		reinterpret_cast<const float*>(&Colors::Blue));
	// Clear the depth buffer to 1.0f and the stencil buffer to 0.
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	// Present the back buffer to the screen.
	HR(mSwapChain->Present(0, 0));
}
