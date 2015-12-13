//--------------------------------------------------------------------------------------
// BTH - Stefan Petersson 2014. All rights reserved.
// Added some comments (Francisco)
//--------------------------------------------------------------------------------------
#include <windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include "DirectXTK-nov2015\Inc\SimpleMath.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

#define WIDTH 640.0f
#define HEIGHT 480.0f
#define PI 3.14159265359f
#define WM_KEYDOWN                      0x0100

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

// Name spaces
using namespace DirectX;
using namespace DirectX::SimpleMath;

// DirectX attributes
IDXGISwapChain* gSwapChain = nullptr;
ID3D11Device* gDevice = nullptr;
ID3D11DeviceContext* gDeviceContext = nullptr;
ID3D11RenderTargetView* gBackbufferRTV = nullptr;
ID3D11DepthStencilView* gZBuffer = nullptr;

ID3D11Texture2D* depthStencilBuffer;

ID3D11Buffer* gVertexBuffer = nullptr;
ID3D11Buffer* indexbuffer = nullptr;
ID3D11Buffer* gCBuffer = nullptr;
ID3D11Buffer* gGeometryBuffer = nullptr;

ID3D11InputLayout* gVertexLayout = nullptr;
ID3D11VertexShader* gVertexShader = nullptr;
ID3D11PixelShader* gPixelShader = nullptr;
ID3D11GeometryShader* gGeometryShader = nullptr;

struct TriangleVertex
{
	float x, y, z;
	float r, g, b;
};

struct OFFSET
{
	float x, y, z;
};

struct COLORMOD
{
	float test[4];
	float test2[4];
	float test3[4];
	float test4[4];
};

float ConvertToRadians(float angle)
{
	return (PI / 180.0f) * angle;
}

void SetViewPortAndDepthBuffer()
{
	HRESULT hr;
	// Create Z-buffer
	D3D11_TEXTURE2D_DESC texzb;
	ZeroMemory(&texzb, sizeof(texzb));

	texzb.Width = WIDTH;
	texzb.Height = HEIGHT;
	texzb.MipLevels = 1;
	texzb.ArraySize = 1;
	texzb.SampleDesc.Count = 1;
	texzb.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texzb.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texzb.Usage = D3D11_USAGE_DEFAULT;
	texzb.CPUAccessFlags = 0;
	texzb.MiscFlags = 0;

	hr = gDevice->CreateTexture2D(&texzb, NULL, &depthStencilBuffer);

	if (FAILED(hr))
		MessageBox(NULL, L"Failed to create depthbuffer.", L"Error", MB_OK);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	ZeroMemory(&dsvd, sizeof(dsvd));

	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	hr = gDevice->CreateDepthStencilView(depthStencilBuffer, &dsvd, &gZBuffer);
	if (FAILED(hr))
		MessageBox(NULL, L"Failed to create depthstencil.", L"Error", MB_OK);


	gDeviceContext->OMSetRenderTargets(1, &gBackbufferRTV, gZBuffer);

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
	ID3DBlob* pVS = nullptr, *vError;

	D3DCompileFromFile(
		L"Vertex.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"VS_main",		// entry point
		"vs_4_0",		// shader model (target)
		0,				// shader compile options
		0,				// effect compile options
		&pVS,			// double pointer to ID3DBlob		
		&vError			// pointer for Error Blob messages.
						// how to use the Error blob, see here
						// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
		);

	if (vError)
		MessageBox(NULL, L"The Vertex shader failed to compile.", L"Error", MB_OK);

	gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &gVertexShader);
	gDeviceContext->VSSetShader(gVertexShader, 0, 0);

	ID3DBlob* pPS = nullptr, *pError;

	D3DCompileFromFile(
		L"Fragment.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"PS_main",		// entry point
		"ps_4_0",		// shader model (target)
		0,				// shader compile options
		0,				// effect compile options
		&pPS,			// double pointer to ID3DBlob		
		&pError		// pointer for Error Blob messages.
		);

	if(pError)
		MessageBox(NULL, L"The Pixel shader failed to compile.", L"Error", MB_OK);

	gDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &gPixelShader);
	gDeviceContext->PSSetShader(gPixelShader, 0, 0);

	ID3DBlob* pGS = nullptr, *gError;

	D3DCompileFromFile(
		L"GeometryShader.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"GS_main",		// entry point
		"gs_4_0",		// shader model (target)
		0,				// shader compile options
		0,				// effect compile options
		&pGS,			// double pointer to ID3DBlob		
		&gError		// pointer for Error Blob messages.
		);

	if (gError)
		MessageBox(NULL, L"The geometry shader failed to compile.", L"Error", MB_OK);

	gDevice->CreateGeometryShader(pGS->GetBufferPointer(), pGS->GetBufferSize(), nullptr, &gGeometryShader);
	gDeviceContext->GSSetShader(gGeometryShader, 0, 0);

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
	HRESULT hr;

	TriangleVertex Vertices[] =
	{
		{ -0.5f, 0.5f, -0.5f,  1.0f, 0.0f, 0.0f },   
		{ 0.5f, 0.5f, -0.5f,   0.0f, 1.0f, 0.0f },    
		{ -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f },  
		{ 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f }    
		//{ -0.5f, 0.5f, 0.5f,   0.0f, 1.0f, 1.0f },   
		//{ 0.5f, 0.5f, 0.5f,    1.0f, 0.0f, 1.0f },
		//{ -0.5f, -0.5f, 0.5f,  1.0f, 1.0f, 0.0f },
		//{ 0.5f, -0.5f, 0.5f,   1.0f, 1.0f, 1.0f },
	};

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));

	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(TriangleVertex) * ARRAYSIZE(Vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA vsrd;
	ZeroMemory(&vsrd, sizeof(vsrd));

	vsrd = { Vertices, 0, 0 };

	hr = gDevice->CreateBuffer(&bufferDesc, &vsrd, &gVertexBuffer);

	if (FAILED(hr))
	{
		MessageBox(NULL, L"Failed to create vertexbuffer.", L"Error", MB_OK);
		hr = NULL;
	}

	D3D11_BUFFER_DESC cbdesc;
	ZeroMemory(&cbdesc, sizeof(cbdesc));

	cbdesc.Usage = D3D11_USAGE_DEFAULT;
	cbdesc.ByteWidth = 64;
	cbdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = gDevice->CreateBuffer(&cbdesc, NULL, &gCBuffer);

	if (FAILED(hr))
	{
		MessageBox(NULL, L"Failed to create constantbuffer.", L"Error", MB_OK);
		hr = NULL;
	}
	
	gDeviceContext->GSSetConstantBuffers(0, 1, &gCBuffer);

	//unsigned int OurIndices[] =
	//{
	//	0, 1, 2,    // side 1
	//	2, 1, 3,
	//	4, 0, 6,    // side 2
	//	6, 0, 2,
	//	7, 5, 6,    // side 3
	//	6, 5, 4,
	//	3, 1, 7,    // side 4
	//	7, 1, 5,
	//	4, 5, 0,    // side 5
	//	0, 5, 1,
	//	3, 7, 2,    // side 6
	//	2, 7, 6,
	//};

	//D3D11_BUFFER_DESC ibd;
	//ibd.ByteWidth = sizeof(unsigned int) * ARRAYSIZE(OurIndices);
	//ibd.Usage = D3D11_USAGE_DEFAULT;
	//ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	//ibd.CPUAccessFlags = 0;
	//ibd.MiscFlags = 0;

	//D3D11_SUBRESOURCE_DATA isrd = { OurIndices, 0, 0 };
	//hr = gDevice->CreateBuffer(&ibd, &isrd, &indexbuffer);


	if (FAILED(hr))
	{
		MessageBox(NULL, L"Failed to create Indexbuffer.", L"Error", MB_OK);
		hr = NULL;
	}

}

void Render()
{
	float color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	static float deltatime;

	Matrix matRotateY;
	Matrix matRotateZ;
	Matrix projection;
	Matrix view;
	Matrix finalMatrix;

	Vector3 cameraPosition(0, 0, -2); 
	Vector3 cameraTarget(0, 0, 0);
	Vector3 cameraUpVector(0, 1, 0);

	deltatime += 0.01;

	float rads = ConvertToRadians(deltatime);

	// Clear the  backbuffer to a blue background
	gDeviceContext->ClearRenderTargetView(gBackbufferRTV, color);

	// Clear the zbuffer
	gDeviceContext->ClearDepthStencilView(gZBuffer, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	// select which primtive type we are using D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// select which vertex buffer to display
	UINT stride = sizeof(TriangleVertex);
	UINT offset = 0;
	gDeviceContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &stride, &offset);
	//gDeviceContext->IASetIndexBuffer(indexbuffer, DXGI_FORMAT_R32_UINT, 0);


	// Create world matrix, projection and view / camera for the first cube
	matRotateY = XMMatrixRotationY(rads);
	/*matRotateZ = XMMatrixRotationZ(rads);*/
	projection = XMMatrixPerspectiveFovLH(PI*0.45f, 1.33f, 0.5f, 20.0f);
	view = XMMatrixLookAtLH(cameraPosition, cameraTarget, cameraUpVector);

    finalMatrix =  matRotateY * view * projection;
	
	// Transpose to get to RH
	finalMatrix = XMMatrixTranspose(finalMatrix);

	// Update constant buffer with the new matrix
	gDeviceContext->UpdateSubresource(gCBuffer, 0, 0, &finalMatrix, 0, 0);


	// draw the vertex buffer to the back buffer
	gDeviceContext->Draw(4, 0); // Draw cube one

	//XMMATRIX mTranslate = XMMatrixTranslation(-2.0f, 0.0, -6.0f);

	//finalMatrix = matRotateZ * mTranslate * matRotateY * view * projection;

	//// Transpose to get to RH
	//finalMatrix = XMMatrixTranspose(finalMatrix);

	//// Update constant buffer with the new matrix
	//gDeviceContext->UpdateSubresource(gCBuffer, 0, 0, &finalMatrix, 0, 0);

	//// draw the vertex buffer to the back buffer
	//gDeviceContext->DrawIndexed(36, 0, 0); // Draw cube two

	// Swap the buffers
	gSwapChain->Present(0, 0);
}

void CleanD3D()
{
	gSwapChain->Release();
	gBackbufferRTV->Release();
	gDevice->Release();
	gDeviceContext->Release();
	gPixelShader->Release();
	gCBuffer->Release();
	depthStencilBuffer->Release();
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
		SetViewPortAndDepthBuffer();

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

	case WM_KEYDOWN:

		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		case 0x41: // A KEY
			break;
		}
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
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	HRESULT hr;

	hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE, // Hardware
		NULL,					  // Handle to software DLL if software driver has been choicen
		D3D11_CREATE_DEVICE_DEBUG,					  // Device flags
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
