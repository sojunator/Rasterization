//--------------------------------------------------------------------------------------
// BTH - Stefan Petersson 2014. All rights reserved.
// Added some comments (Francisco)
//--------------------------------------------------------------------------------------
#include <windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

#define WIDTH 640;
#define HEIGHT 480;

HWND InitWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/*
 Entry point for our program
 This part of the code uses extensively Windows Datatypes, please the following
 link for more detail about them,
 https://msdn.microsoft.com/en-us/library/windows/desktop/aa383751(v=vs.85).aspx
*/

// Prototypes
HRESULT CreateDirect3DContext(HWND wndHandle);


// DirectX attributes
IDXGISwapChain* gSwapChain = nullptr;
ID3D11Device* gDevice = nullptr;
ID3D11DeviceContext* gDeviceContext = nullptr;
ID3D11RenderTargetView* gBackbufferRTV = nullptr;

ID3D11Buffer* gVertexBuffer = nullptr;

ID3D11InputLayout* gVertexLayout = nullptr;
ID3D11VertexShader* gVertexShader = nullptr;
ID3D11PixelShader* gPixelShader = nullptr;

struct TriangleVertex
{
	float x, y, z;
	float r, g, b;
};

void SetViewPort()
{
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.Width = WIDTH;
	viewport.Height = HEIGHT;

	gDeviceContext->RSSetViewports(1, &viewport);
}

void CreateShaders()
{
	ID3DBlob* pVS = nullptr;
	D3DCompileFromFile(
		L"Vertex.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"VS_main",		// entry point
		"vs_4_0",		// shader model (target)
		0,				// shader compile options
		0,				// effect compile options
		&pVS,			// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
						// how to use the Error blob, see here
						// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
		);

	gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &gVertexShader);
	gDeviceContext->VSSetShader(gVertexShader, 0, 0);

	ID3DBlob* pPS = nullptr;

	D3DCompileFromFile(
		L"Fragment.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"PS_main",		// entry point
		"ps_4_0",		// shader model (target)
		0,				// shader compile options
		0,				// effect compile options
		&pPS,			// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
		);

	gDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &gPixelShader);
	gDeviceContext->PSSetShader(gPixelShader, 0, 0);

	// Input layout object
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{ 
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	gDevice->CreateInputLayout(ied, ARRAYSIZE(ied), pVS->GetBufferPointer(), pVS->GetBufferSize(), &gVertexLayout);

	gDeviceContext->IASetInputLayout(gVertexLayout);
}

void CreateTriangleData()
{


	TriangleVertex first_triangle[5] =
	{
		0.5f, 0.5f, 0.0f,	//v0 pos
		1.0f, 0.0f, 0.0f,	//v0 color

		0.5f, -0.5f, 0.0f,	//v1
		0.0f, 1.0f, 0.0f,	//v1 color

		-0.5f, -0.5f, 0.0f, //v2
		0.0f, 0.0f, 1.0f,	//v2 color

		-0.5f, 0.5f, 0.0f,   //v3
		0.7f, 0.4f, 0.3f,	//v2 color

		0.5f, 0.5f, 0.0f,	//v0 pos
		1.0f, 0.0f, 0.0f,	//v0 color

	};


	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));

	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(TriangleVertex) * 5;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	gDevice->CreateBuffer(&bufferDesc, NULL, &gVertexBuffer);

	D3D11_MAPPED_SUBRESOURCE ms;
	gDeviceContext->Map(gVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, first_triangle, sizeof(first_triangle));
	gDeviceContext->Unmap(gVertexBuffer, NULL);
}

void Render()
{
	float color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	gDeviceContext->ClearRenderTargetView(gBackbufferRTV, color);

	// select which vertex buffer to display
	UINT stride = sizeof(TriangleVertex);
	UINT offset = 0;
	gDeviceContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &stride, &offset);

	// select which primtive type we are using D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// draw the vertex buffer to the back buffer
	gDeviceContext->Draw(5, 0);

	gSwapChain->Present(0, 0);
}

void CleanD3D()
{
	gSwapChain->Release();
	gBackbufferRTV->Release();
	gDevice->Release();
	gDeviceContext->Release();
	gPixelShader->Release();
	gVertexBuffer->Release();
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	MSG msg = { 0 };

	// create window
	HWND wndHandle = InitWindow(hInstance);
	
	// window is valid
	if (wndHandle)
	{
		// display window
		ShowWindow(wndHandle, nCmdShow);

		// Create Device and swapchain
		CreateDirect3DContext(wndHandle);

		// Create and set viewport
		SetViewPort(); 

		//
		CreateShaders();


		CreateTriangleData(); //5. Definiera triangelvertiser, 6. Skapa vertex buffer, 7. Skapa input layout

		// enter message loop, loop until the message WM_QUIT is received.
		while (WM_QUIT != msg.message)
		{
			// read messages
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				// this will call the function WndProc below!
				DispatchMessage(&msg);
			}
			else
			{
				Render();
			}
		}

		// finish the program
		CleanD3D();
		DestroyWindow(wndHandle);
	}

	// return how the program finished.
	return (int) msg.wParam;
}

HWND InitWindow(HINSTANCE hInstance)
{
	// Every window created must belong to a CLASS, so
	// we first create a new class.
	// fill in the WNDCLASSEX structure
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;			// Which function is called for events
	wcex.hInstance      = hInstance;
	wcex.lpszClassName  = L"BasicWindow";
	// use the struct to register the new class.
	// the name of the class is "BasicWindow"
	if( !RegisterClassEx(&wcex) )
		return false;

	// define a struct for the window size
	RECT rc = { 0, 0, 640, 480 };
	// create the window to this size and with additional properties
	// (border, menu, etc)
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	
	HWND handle = CreateWindow(
		L"BasicWindow",			// CLASS, if does not exists fails!
		L"hl3.exe",		// Window name (title)
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,		// width
		rc.bottom - rc.top,		// height
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	return handle;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message) 
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;		
	}

	// if we do not handle the message here, simply call the Default handler function
	return DefWindowProc(hWnd, message, wParam, lParam);
}

HRESULT CreateDirect3DContext(HWND wndHandle)
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));

	sd.BufferCount = 1;
	//sd.BufferDesc.Width = WIDTH;
	//sd.BufferDesc.Height = HEIGHT;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = wndHandle;
	sd.SampleDesc.Count = 4;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	HRESULT hr;

	hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE, // Hardware
		NULL,					  // Handle to software DLL if software driver has been choicen
		NULL,					  // Device flags
		NULL,					  // Feature levels
		NULL,					  // Feature levels
		D3D11_SDK_VERSION,		  //  SDK version
		&sd,					  // Our swapchaindesc
		&gSwapChain,
		&gDevice,
		NULL,					  // Feature level
		&gDeviceContext
	);

	if (SUCCEEDED(hr))
	{
		ID3D11Texture2D* pBackBuffer = nullptr;
		gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		gDevice->CreateRenderTargetView(pBackBuffer, NULL, &gBackbufferRTV);
		pBackBuffer->Release();

		gDeviceContext->OMSetRenderTargets(1, &gBackbufferRTV, NULL);

	}

	return hr;
}

