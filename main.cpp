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

void MyD3DApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}


void MyD3DApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void MyD3DApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));
		
		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;
		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);
		// Update the camera radius based on input.
		mRadius += dx - dy;
		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void MyD3DApp::BuildGeometryBuffers()
{
	// Create vertex buffer
	VERTEX vertices[] =
	{
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4((const float*)&Colors::White) },
	{ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Black) },
	{ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Red) },
	{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Green) },
	{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Blue) },
	{ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Yellow) },
	{ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Cyan) },
	{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Magenta) }
	};

	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.ByteWidth = sizeof(VERTEX) * 8;
	vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = vertices;
	HR(md3dDevice->CreateBuffer(&vertexBufferDesc, &initData, &mBoxVertexBuffer));

	UINT indices[] = {
		// front face
		0, 1, 2,
		0, 2, 3,
		// back face
		4, 6, 5,
		4, 7, 6,
		// left face
		4, 5, 1,
		4, 1, 0,
		// right face
		3, 2, 6,
		3, 6, 7,
		// top face
		1, 5, 6,
		1, 6, 2,
		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = sizeof(UINT) * 36;
	indexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA indexInitData;
	indexInitData.pSysMem = indices;
	HR(md3dDevice->CreateBuffer(&indexBufferDesc, &indexInitData, &mBoxIndexBuffer));
}

void MyD3DApp::BuildFX()
{
	DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob* compiledShader = 0;
	ID3D10Blob* compilationMsgs = 0;
	HRESULT hr = D3DX11CompileFromFile(L"./color.fx", 0, 0, 0, "fx_5_0", shaderFlags, 0, 0, &compiledShader, &compilationMsgs, 0);

	// compilationMsgs can store errors or warnings.
	if (compilationMsgs != 0)
	{
		MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
		ReleaseCOM(compilationMsgs)
	}

	//Even if there were no compilationMsgs, make sure there are no errors.
	if (FAILED(hr))
	{
		DXTrace(__FILEW__, (DWORD)__LINE__, hr, L"D3DX11CompileFromFile", true);
	}

	HR(D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), 0, md3dDevice, &mEffect));

	// Done with the compiled shader
	ReleaseCOM(compiledShader);

	mEffectTechnique = mEffect->GetTechniqueByName("CoorTech");
	mEffectWorldViewProjMatrix = mEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
}

void MyD3DApp::BuildVertexLayout()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// create input layout
	D3DX11_PASS_DESC passDesc;
	mEffectTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout));
}

