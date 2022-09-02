#include <ctime>
#include <math.h>
#include <Windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>

//global declarations
IDXGISwapChain* swapChain;
ID3D11Device* dev;
ID3D11DeviceContext* devcon;
ID3D11RenderTargetView* backBuffer;

//function prototypes
void InitD3D(HWND hWnd);
void CleanD3D(void);
void RenderFrame(void);

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
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
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
							300,							//x position of the window
							300,							//y position of the window
							wr.right - wr.left,							//width of the window
							wr.bottom - wr.top,							//height of the window
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
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 4;
	scd.Windowed = TRUE;

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
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = 800;
	viewport.Height = 600;

	devcon->RSSetViewports(1, &viewport);
	#pragma endregion

}

void CleanD3D()
{
	dev->Release();
	swapChain->Release();
	devcon->Release();
	backBuffer->Release();
}

void RenderFrame()
{
	//clear render target to color
	devcon->ClearRenderTargetView(backBuffer, D3DXCOLOR(.8f, .6f, .4f, 1.0f));

	//render stuff here

	//swap the buffers
	swapChain->Present(0, 0);
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
