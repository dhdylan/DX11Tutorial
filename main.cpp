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

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();

private:
	ID3D11Buffer* mBoxVertexBuffer;
	ID3D11Buffer* mBoxIndexBuffer;

	ID3DX11Effect* mEffect;
	ID3DX11EffectTechnique* mEffectTechnique;
	ID3DX11EffectMatrixVariable* mEffectWorldViewProjMatrix;

	ID3D11InputLayout* mInputLayout;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
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
	: D3DApp(hInstance), mBoxVertexBuffer(0), mBoxIndexBuffer(0), mEffect(0), mEffectTechnique(0), mEffectWorldViewProjMatrix(0), mInputLayout(0), mTheta(1.5 * MathHelper::Pi), mPhi(.25 * MathHelper::Pi), mRadius(5)
{
	mMainWndCaption = L"Box Demo";
	mLastMousePos.x = 0.0;
	mLastMousePos.y = 0.0;

	XMMATRIX identityMatrix = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, identityMatrix);
	XMStoreFloat4x4(&mView, identityMatrix);
	XMStoreFloat4x4(&mProj, identityMatrix);
}

MyD3DApp::~MyD3DApp()
{
	ReleaseCOM(mBoxVertexBuffer);
	ReleaseCOM(mBoxIndexBuffer);
	ReleaseCOM(mEffect);
	ReleaseCOM(mInputLayout);
}

bool MyD3DApp::Init()
{
	if (!D3DApp::Init())
		return false;
	return true;

	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	return true;
}

void MyD3DApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(.25 * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void MyD3DApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	//generate view matrix
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0, 1.0, 0.0, 0.0);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);	
}

void MyD3DApp::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);
	// Clear the back buffer blue. Colors::Blue is defined in d3dUtil.h.
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView,
		reinterpret_cast<const float*>(&Colors::White));
	// Clear the depth buffer to 1.0f and the stencil buffer to 0.
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Set pipeline states for drawing box
	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVertexBuffer, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mBoxIndexBuffer, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

	//set shader constants
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX wvp = world * view * proj;

	mEffectWorldViewProjMatrix->SetMatrix(reinterpret_cast<float*>(&wvp));

	// draw from technique
	D3DX11_TECHNIQUE_DESC techniqueDesc;
	mEffectTechnique->GetDesc(&techniqueDesc);

	for (UINT p = 0; p < techniqueDesc.Passes; ++p)
	{
		mEffectTechnique->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		md3dImmediateContext->DrawIndexed(36, 0, 0);
	}

	// Present the back buffer to the screen.
	HR(mSwapChain->Present(0, 0));
}
