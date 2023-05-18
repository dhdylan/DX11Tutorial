#include "d3dApp.h"
#include <MathHelper.h>
#include <d3dx11effect.h>
#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include <string>

struct VERTEX {
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

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
