#include <Windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

struct VERTEX
{
	float x, y, z;
	D3DXCOLOR color;
};


//global declarations
IDXGISwapChain* swapChain;
ID3D11Device* dev;
ID3D11DeviceContext* devcon;
ID3D11RenderTargetView* backBuffer;
ID3D11VertexShader* vertexShader;
ID3D11PixelShader* pixelShader;
ID3D11Buffer* vertexBuffer;
ID3D11InputLayout* inputLayout;

//function prototypes
void InitD3D(HWND hWnd);
void CleanD3D(void);
void RenderFrame(void);
void InitPipeline(void);
void InitGraphics(void);

//the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd,	UINT message,	WPARAM wParam,	LPARAM lParam);

//entry point for any windows program
int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR lpCmdLine,
					int nCmdShow)
{
	//handle for the window
	HWND hWnd;
	//this struct holds information for the window class
	WNDCLASSEX wc;

	//clear out the window class for use
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	//fill in the fields of the struct
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	//wc.hbrBackground = (HBRUSH)COLOR_WINDOW; //removing the bakcground color so that when the app launches it doesn't flush the screen with this 
	wc.lpszClassName = L"WindowClass1";

	//register this window class
	RegisterClassEx(&wc);

	RECT wr = { 0, 0, 500, 400 };	//set the size but not the position
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE); //adjust that size


	//create the window from that class and store the handle
	hWnd = CreateWindowEx(	NULL,							
							L"WindowClass1",				//name of the window class
							L"My first windowed program",	//title of the window
							WS_OVERLAPPEDWINDOW,			//window style
							0,							//x position of the window
							0,							//y position of the window
							SCREEN_WIDTH,							//width of the window
							SCREEN_HEIGHT,							//height of the window
							NULL,							//we have no parent window, NULL
							NULL,							//we aren't using menus, NULL
							hInstance,						//handle to the application instance
							NULL);							//used with multiple windows, NULL

	//display the window on the screen
	ShowWindow(hWnd, nCmdShow);

	//this struct holds Windows event messages
	MSG msg;

	InitD3D(hWnd);




	/////////////
	//MAIN LOOP//
	/////////////
	while (TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			//translate keystroke message to the right format
			TranslateMessage(&msg);

			//send the message to the WindowProc function
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;
		}
		else
		{
			//do game stuff
			RenderFrame();
		}
	}

	CleanD3D();


	return msg.wParam;
}

void InitD3D(HWND hWnd)
{
	#pragma region Initialize D3D
	//create a swap chain description struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	//clear out memory
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	//fill out the struct
	scd.BufferCount = 1;
	scd.BufferDesc.Width = SCREEN_WIDTH;
	scd.BufferDesc.Height = SCREEN_HEIGHT;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 4; //how many samples for antialiasing ?
	scd.Windowed = TRUE;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; //allow fullscreen switching.

	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL, D3D11_SDK_VERSION, &scd, &swapChain, &dev, NULL, &devcon);
	#pragma endregion

	#pragma region Set Render Target
	//get the address of the back buffer
	ID3D11Texture2D* pBackBuffer;
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	dev->CreateRenderTargetView(pBackBuffer, NULL, &backBuffer);
	devcon->OMSetRenderTargets(1, &backBuffer, NULL);

	pBackBuffer->Release();
	#pragma endregion

	#pragma region Set viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;

	devcon->RSSetViewports(1, &viewport);
	#pragma endregion

	InitPipeline();
	InitGraphics();

}

void CleanD3D()
{
	swapChain->SetFullscreenState(false, NULL);

	inputLayout->Release();
	vertexShader->Release();
	pixelShader->Release();
	vertexBuffer->Release();
	swapChain->Release();
	dev->Release();
	devcon->Release();
	backBuffer->Release();
}

void RenderFrame()
{
	//clear render target to color
	devcon->ClearRenderTargetView(backBuffer, D3DXCOLOR(.8f, .6f, .4f, 1.0f));

	//render stuff here

	//tell the context what buffer we're about to draw
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	devcon->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	//what primitive are we drawing?
	devcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//draw to back buffer
	devcon->Draw(3, 0);

	//swap the buffers
	swapChain->Present(0, 0);
}



void InitPipeline()
{
	//this is the memory where the compiled shader lives
	//compile the shader and put it in VVVVV memory "blob"
	ID3D10Blob *VS, *PS;
	D3DX11CompileFromFile(L"shaders.shader", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &VS, 0, 0);
	D3DX11CompileFromFile(L"shaders.shader", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &PS, 0, 0);

	//creates shader objects
	dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &vertexShader);
	dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pixelShader);

	devcon->VSSetShader(vertexShader, 0, 0);
	devcon->PSSetShader(pixelShader, 0, 0);

	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	dev->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &inputLayout);
	devcon->IASetInputLayout(inputLayout);
}

void InitGraphics()
{
	VERTEX vertices[] =
	{
		{0.0f, 0.5f, 0.0f, D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f)},
		{0.45f, -0.5, 0.0f, D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f)},
		{-0.45f, -0.5f, 0.0f, D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f)}
	};


	//creating a vertexBuffer to hold the vertices V V V V V
	//make a buffer description struct to hold some info on  the buffer we're making
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.ByteWidth = sizeof(VERTEX) * 3; //size of buffer
	bd.Usage = D3D11_USAGE_DYNAMIC; //usage flag
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; //let CPU write to the buffer
	
	dev->CreateBuffer(&bd, NULL, &vertexBuffer); //create buffer on GPU

	//copy data into buffer
	D3D11_MAPPED_SUBRESOURCE ms;
	devcon->Map(vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms); //map the buffer so the GPU cant touch it while the CPU is writing to it
	memcpy(ms.pData, vertices, sizeof(vertices)); //literally copy the data
	devcon->Unmap(vertexBuffer, NULL); //unmap, GPU is allowed to look again
}

//this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//sort through and find what code to run for the message
	switch (message)
	{
		//this message is read when the window is closed
		case WM_DESTROY:
		{
			//close the application entirely
			PostQuitMessage(0);
			return 0;
		} break;
	}

	//handle any messages the switch statement didn't
	return DefWindowProc(hWnd, message, wParam, lParam);
}
