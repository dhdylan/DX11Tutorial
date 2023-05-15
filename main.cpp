#include "d3dApp.h"
#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include <string>

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
		DXGI_ADAPTER_DESC adapterDesc;
		ZeroMemory(&adapterDesc, sizeof(DXGI_ADAPTER_DESC));

		adapter->GetDesc(&adapterDesc);
		Print(L"****************************\nAdapter description:\t\t");
		PrintLn(adapterDesc.Description);

		LARGE_INTEGER li;
		if (adapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &li) == S_OK)
		{
			Print(L"Supported:\t\t\t\t\t");
			PrintLn(L"DEVICE SUPPORTED");
		}
		else
		{
			Print(L"Supported:\t\t");
			PrintLn(L"DEVICE NOT SUPPORTED");
		}

		PrintLn(L"Adapter Outputs:");
		IDXGIOutput* output = 0;

		int j = 0;

		while (adapter->EnumOutputs(j, &output) != DXGI_ERROR_NOT_FOUND)
		{
			// Print output device
			DXGI_OUTPUT_DESC outputDesc;
			output->GetDesc(&outputDesc);
			Print(L"\t\t");
			PrintLn(outputDesc.DeviceName);

			// Print display modes for this device
			UINT numModes = 0;
			output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, NULL);
			DXGI_MODE_DESC* modeDesc= static_cast<DXGI_MODE_DESC*>(calloc(numModes, sizeof(DXGI_MODE_DESC)));
			output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, modeDesc);

			for (UINT k = 0; k < (UINT)numModes; k++)
			{
				Print(L"\t\t\t\tWIDTH: ");
				Print(std::to_wstring(modeDesc[k].Width).c_str());
				Print(L"\t\t\tHEIGHT: ");
				Print(std::to_wstring(modeDesc[k].Height).c_str());
				Print(L"\t\t\tREFRESH: ");
				PrintLn(std::to_wstring(modeDesc[k].RefreshRate.Numerator / modeDesc->RefreshRate.Denominator).c_str());
			}

			free(modeDesc);
			ReleaseCOM(output);
			j++;
		}

		if (j < 1)
			PrintLn(L"\t\tNO OUTPUTS FOUND");

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
