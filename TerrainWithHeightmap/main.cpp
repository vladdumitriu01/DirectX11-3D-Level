#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <stdio.h>
#include <WICTextureLoader.h>
#include "KeyProcessor.h"

//Assimp Include Files
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>


// Direct X Header Files
// Direct X Library Files
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "directxtk.lib")
#pragma comment (lib, "assimp-vc140-mt.lib") 
using namespace DirectX;
using namespace std;

static wchar_t szAppClassName[] = L"VladDumitriu Final Presentation S16102144";


// Declares identifiers
LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


ID3D11Device *d3dDevice;
ID3D11DeviceContext *d3dContext;
IDXGISwapChain *swapChain; // Lets the window run
ID3D11RenderTargetView *backBuffer; // What we render things onto
ID3D11Texture2D* depthStencilBuffer; // The textures
ID3D11DepthStencilView* depthStencilView;

// Constant Buffer used for models in the scene

struct ConstantBuffer
{

	XMMATRIX mWorld; // The scene
	XMMATRIX mView;
	XMMATRIX mProjection;

};



struct theTerrain
{

	XMFLOAT3 pos;
	XMFLOAT2 texCoord;

	theTerrain(XMFLOAT3 xyz, XMFLOAT2 uv)
	{

		pos = xyz; // Keeps the position
		texCoord = uv; // Keeps the position of textures

	}

	theTerrain()
	{

		pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		texCoord = XMFLOAT2(0.0, 0.0f);

	}

};


struct theModel
{

	XMFLOAT3 pos; // Keeps the model position
	XMFLOAT2 texCoord; // Keeps texture position

	theModel() {

	}

	theModel(float x, float y, float z, float u, float v)
	{

		pos.x = x; // x
		pos.y = y; // y
		pos.z = z; // z

		texCoord.x = u;
		texCoord.y = v;

	}

};

struct FogCb {
	XMFLOAT3 eyePos;
	float fogStart;
	float fogRange;

	XMFLOAT3 pad;
	XMFLOAT4 fogColour;
};
XMFLOAT3 skybox[] = // An array that holds all of the vertices we need to make the skybox
{
	// Front

	XMFLOAT3(-1.0f, -1.0f, -1.0f),
	XMFLOAT3(-1.0f, 1.0f, -1.0f),
	XMFLOAT3(1.0f, -1.0f, -1.0f),

	XMFLOAT3(-1.0f, 1.0f, -1.0f),
	XMFLOAT3(1.0f, 1.0f, -1.0f),
	XMFLOAT3(1.0f, -1.0f, -1.0f),

	// Right
	XMFLOAT3(1.0f, 1.0f, -1.0f),
	XMFLOAT3(1.0f, 1.0f, 1.0f),
	XMFLOAT3(1.0f, -1.0f, 1.0f),

	XMFLOAT3(1.0f, -1.0f, 1.0f),
	XMFLOAT3(1.0f, -1.0f, -1.0f),
	XMFLOAT3(1.0f, 1.0f, -1.0f),

	// Back - Counter Clockwise Order
	XMFLOAT3(-1.0f, -1.0f, 1.0f),
	XMFLOAT3(1.0f, -1.0f, 1.0f),
	XMFLOAT3(1.0f, 1.0f, 1.0f),

	XMFLOAT3(1.0f, 1.0f, 1.0f),
	XMFLOAT3(-1.0f, 1.0f, 1.0f),
	XMFLOAT3(-1.0f, -1.0f, 1.0f),

	//// Left - Counter Clockwise Order
	XMFLOAT3(-1.0f, 1.0f, 1.0f),
	XMFLOAT3(-1.0f, 1.0f, -1.0f),
	XMFLOAT3(-1.0f, -1.0f, -1.0f),

	XMFLOAT3(-1.0f, -1.0f, -1.0f),
	XMFLOAT3(-1.0f, -1.0f, 1.0f),
	XMFLOAT3(-1.0f, 1.0f, 1.0f),

	// Top
	XMFLOAT3(-1.0f, 1.0f, -1.0f),
	XMFLOAT3(-1.0f, 1.0f, 1.0f),
	XMFLOAT3(1.0f, 1.0f, 1.0f),

	XMFLOAT3(1.0f, 1.0f, 1.0f),
	XMFLOAT3(1.0f, 1.0f, -1.0f),
	XMFLOAT3(-1.0f, 1.0f, -1.0f),

	// Bottom - Counter Clockwise Order
	XMFLOAT3(-1.0f, -1.0f, -1.0f),
	XMFLOAT3(1.0f, -1.0f, -1.0f),
	XMFLOAT3(1.0f, -1.0f, 1.0f),

	XMFLOAT3(1.0f, -1.0f, 1.0f),
	XMFLOAT3(-1.0f, -1.0f, 1.0f),
	XMFLOAT3(-1.0f, -1.0f, -1.0f),

};



ID3D11Buffer *model1; // Holds the buffer for each model
ID3D11ShaderResourceView *modelOne; // Shaders for each model

ID3D11Buffer *model2;
ID3D11ShaderResourceView *modelTwo;

ID3D11Buffer *model3;
ID3D11ShaderResourceView *modelThree;

ID3D11Buffer *model4;
ID3D11ShaderResourceView *modelFour;

ID3D11Buffer *model5;
ID3D11ShaderResourceView *modelFive;

ID3D11Buffer *fogConstantBuffer = NULL;

ID3D11Buffer *vertexBuffer = NULL; // Not rendering anything by default
ID3D11Buffer *constantBuffer = NULL; // Not rendering anything by default
ID3D11InputLayout *vertexLayout = NULL; // Not rendering anything by default
ID3D11VertexShader *vertexShader;
ID3D11PixelShader *pixelShader;

ID3D11ShaderResourceView *imageData;
ID3D11SamplerState *sampler;



ID3D11Buffer *skyboxVertexBuffer = NULL;
ID3D11VertexShader *skyboxVertexShader = NULL;
ID3D11PixelShader *skyboxPixelShader = NULL;
ID3D11ShaderResourceView *cubeMap;
ID3D11InputLayout *skyboxVertexLayout;



ID3D11RasterizerState *rsCullingOff;
ID3D11RasterizerState *rsCullingOn;
ID3D11DepthStencilState *DSDepthOff;
ID3D11DepthStencilState *DSDepthOn;


XMMATRIX matWorld; 
XMMATRIX matView;
XMMATRIX matProjection;


float camXPos = 0.0f;
float camYPos = 5.0f;
float camZPos = -50.0f;

XMVECTOR camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

float camRotX = 0.0f;
float camRotY = 0.0f;
int curMouseXPos, curMouseYPos; 

int clientMidX, clientMidY; // Middle of user's screen for reference
int winMidX, winMidY;
int winLeft, winTop;

float rotFactorY = 0.0f;
DWORD frameTime;



ID3D11Buffer *terrainVertexBuffer = NULL;
ID3D11Buffer *terrainIndexBuffer = NULL;

// Variables to store vertices for models

int NumIndices;  
int NumVertices; 
int NumVertices2;
int NumVertices3;
int NumVertices4;
int NumVertices5;

ID3D11ShaderResourceView *terrainTexture;


BOOL InitialiseDirectX(HWND hMainWnd, HINSTANCE hCurInstance)
{

	RECT rectDimensions; 
	GetClientRect(hMainWnd, &rectDimensions); // Passes Rect Dimensions

											  // Creates the Window

	LONG width = rectDimensions.right - rectDimensions.left;
	LONG height = rectDimensions.bottom - rectDimensions.top;

	RECT rectWindow;
	GetWindowRect(hMainWnd, &rectWindow);

	winTop = rectWindow.top;
	winLeft = rectWindow.left;

	winMidX = (rectWindow.right - rectWindow.left) / 2;
	winMidY = (rectWindow.bottom - rectWindow.top) / 2;

	clientMidX = width / 2;
	clientMidY = height / 2;

	curMouseXPos = clientMidX; // Keeps mouse position
	curMouseYPos = clientMidY; 

	SetCursorPos(winLeft + winMidX, winTop + winMidY); 
	rotFactorY = XM_PIDIV2 / width;

	D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_1 };
	int numFeatureLevel = sizeof(featureLevel) / sizeof(D3D_FEATURE_LEVEL);

	XMVECTOR camPos = XMVectorSet(camXPos, camYPos, camZPos, 0.0f);
	matView = XMMatrixLookAtLH(camPos, camTarget, camUp);

	matProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2 / 2.0f, width
		/ (FLOAT)height, 0.01f, 10000.0f);


	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hMainWnd; 
	swapChainDesc.Windowed = true; 
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	HRESULT result;
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL, D3D11_SDK_VERSION,
		&swapChainDesc, &swapChain, &d3dDevice, NULL, &d3dContext);

	

	if (result != S_OK) { 
		MessageBox(hMainWnd, TEXT("Failed to initialise DX11!"), szAppClassName, NULL);
		return false;
	}

	

	ID3D11Texture2D *backBufferTexture;
	result = swapChain->GetBuffer(0, _uuidof(ID3D11Texture2D), (LPVOID *)&backBufferTexture);

	if (result != S_OK) {
		MessageBox(hMainWnd, TEXT("Failed to get back buffer!"), szAppClassName, NULL);

		return false;
	}

	result = d3dDevice->CreateRenderTargetView(backBufferTexture, 0, &backBuffer);

	if (backBufferTexture != NULL) {
		backBufferTexture->Release();
	}

	if (result != S_OK) {
		MessageBox(hMainWnd, TEXT("Failed to get render target!"), szAppClassName, NULL);

		return false;
	}

	d3dContext->OMSetRenderTargets(1, &backBuffer, 0);



	D3D11_TEXTURE2D_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_TEXTURE2D_DESC));


	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;

	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;


	d3dDevice->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);

	d3dDevice->CreateDepthStencilView(depthStencilBuffer,
		NULL, &depthStencilView);

	d3dContext->OMSetRenderTargets(1, &backBuffer, depthStencilView);



	D3D11_VIEWPORT viewport; 

	viewport.Width = static_cast<float>(width); 
	viewport.Height = static_cast<float>(height); 

	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	d3dContext->RSSetViewports(1, &viewport);

	XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -10.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 3.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	matView = XMMatrixLookAtLH(Eye, At, Up);

	return true;

}


bool LoadFace(const wchar_t *filename, int faceIdx, ID3D11Texture2D *cubemapTex)
{


	ID3D11ShaderResourceView *faceData;
	ID3D11Resource *faceRes;
	HRESULT hr = CreateWICTextureFromFile(d3dDevice, d3dContext,
		filename, &faceRes, &faceData, 0);

	if (hr != S_OK) {
		return false;
	}

	ID3D11Texture2D *tex; 

	hr = faceRes->QueryInterface(__uuidof(ID3D11Texture2D), (LPVOID *)&tex);

	if (hr != S_OK) {
		return false;
	}

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	tex->GetDesc(&texDesc);

	D3D11_BOX srcRegion;

	srcRegion.front = 0;
	srcRegion.back = 1;
	srcRegion.top = 0;
	srcRegion.left = 0;
	srcRegion.bottom = texDesc.Height;
	srcRegion.right = texDesc.Width;

	int face = D3D11CalcSubresource(0, faceIdx, 1);
	d3dContext->CopySubresourceRegion(cubemapTex, face, 0, 0, 0, faceRes,
		0, &srcRegion);

	faceData->Release();
	return true;

}


bool CreateCubemapSkybox(const wchar_t *up_fname, const wchar_t *down_fname, const wchar_t *left_fname,
	const wchar_t *right_fname, const wchar_t *front_fname, const wchar_t *back_fname, int lengthOfSide)
{

	D3D11_TEXTURE2D_DESC cubemapDesc;
	ZeroMemory(&cubemapDesc, sizeof(D3D11_TEXTURE2D_DESC));

	cubemapDesc.Width = lengthOfSide;
	cubemapDesc.Height = lengthOfSide;
	cubemapDesc.MipLevels = 1;
	cubemapDesc.ArraySize = 6;
	cubemapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	cubemapDesc.SampleDesc.Count = 1;
	cubemapDesc.SampleDesc.Quality = 0;
	cubemapDesc.Usage = D3D11_USAGE_DEFAULT;
	cubemapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	cubemapDesc.CPUAccessFlags = 0;
	cubemapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	ID3D11Texture2D *cubemapBuffer;

	HRESULT hr = d3dDevice->CreateTexture2D(&cubemapDesc, nullptr, &cubemapBuffer);

	if (hr != S_OK) { // If hr doesn't load correctly, error
		return false;
	}

	LoadFace(front_fname, 4, cubemapBuffer);
	LoadFace(up_fname, 2, cubemapBuffer);
	LoadFace(down_fname, 3, cubemapBuffer);
	LoadFace(left_fname, 1, cubemapBuffer);
	LoadFace(right_fname, 0, cubemapBuffer);
	LoadFace(back_fname, 5, cubemapBuffer);

	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
	ZeroMemory(&srDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	srDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srDesc.Texture2D.MostDetailedMip = 0;
	srDesc.Texture2D.MipLevels = 1;


	hr = d3dDevice->CreateShaderResourceView(cubemapBuffer, &srDesc, &cubeMap);

	if (hr != S_OK) {
		return false;
	}
	cubemapBuffer->Release();

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(XMFLOAT3) * ARRAYSIZE(skybox);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = skybox;
	hr = d3dDevice->CreateBuffer(&bd, &InitData, &skyboxVertexBuffer);
	if (hr != S_OK) {
		return false;
	}

	return true;

}

void LoadHeightMap(const char *filename, float MaxHeight, theTerrain **mesh) {
	// This example expects a texture file of 256 pixels width and height
	// More flexible version would obtain the width and height from
	// file and create terrain from this data as terrain vertices
	// map onto pixel data
	// Also presumes height map is 24 bit texture file
	const int Size = 256;

	FILE *file_ptr = fopen(filename, "rb");

	if (file_ptr == NULL) {
		return;
	}

	// Read the file's header information
	// so we can create a buffer to hold the pixel colour data
	BITMAPFILEHEADER bmHeader;
	BITMAPINFOHEADER bmInfo;

	fread(&bmHeader, sizeof(BITMAPFILEHEADER), 1, file_ptr);

	fread(&bmInfo, sizeof(BITMAPINFOHEADER), 1, file_ptr);

	// Move file pointer to start of pixel colour data
	fseek(file_ptr, bmHeader.bfOffBits, SEEK_SET);

	// Work out number of bytes per pixel for array
	int NumBytesPerPixel = bmInfo.biBitCount / 8;

	// Calculate maximum colour value up 2^24 colour
	float MaxColourVal = (float)pow(2.0f, min(24.0f, (float)bmInfo.biBitCount));

	// Create array for pixel data
	unsigned char *pixel_data = new unsigned char[Size * Size * NumBytesPerPixel];

	// Read pixel data from file into memory
	fread(pixel_data, sizeof(unsigned char), Size * Size * NumBytesPerPixel, file_ptr);

	// Release the file
	fclose(file_ptr);

	//Modify each vertex's y component based on the pixel value in proportion to max colour value
	// in the corresponding pixel
	int idx = 0;
	theTerrain *vertArray = *mesh;
	for (int row = 0; row < Size; row++) {
		for (int col = 0; col < Size; col++) {
			int pixel_val = pixel_data[idx * NumBytesPerPixel]
				+ (pixel_data[(idx *NumBytesPerPixel) + 1] << 8)
				+ (pixel_data[(idx *NumBytesPerPixel) + 2] << 16);

			vertArray[idx].pos.y = MaxHeight * (pixel_val / MaxColourVal);
			idx++;

		}
	}

	// Release pixel data as don't need it any more
	delete[] pixel_data;
}

void CreateTerrainMesh(int NumVertices, theTerrain **mesh, int *TotalNumVert,
	int **indices, int *NumIndices)

{

	*TotalNumVert = NumVertices * NumVertices;
	*mesh = new theTerrain[*TotalNumVert];

	float step = 2.0f / NumVertices;

	float xPos = -1.0f;
	float zPos = -1.0f;

	float uvStep = 1.0f / NumVertices;

	float u = 0.0f;
	float v = 0.0f;

	int idx = 0;
	theTerrain *vertArray = *mesh;
	
	for (int row = 0; row < NumVertices; row++)
	{

		for (int col = 0; col < NumVertices; col++)
		{

			vertArray[idx] = theTerrain(XMFLOAT3(xPos, 0.0f, zPos), XMFLOAT2(u, v));

			xPos += step;
			u = +uvStep;
			idx++;

		}

		u = 0.0f;
		v += uvStep;
		xPos = -1.0f;
		zPos += step;

	}
	//create the index array
	*NumIndices = (NumVertices - 1) * (NumVertices - 1) * 6;
	*indices = new int[*NumIndices];

	int *p = *indices;

	p[0] = 0;
	p[1] = NumVertices;
	p[2] = NumVertices + 1;
	p[3] = NumVertices + 1;
	p[4] = 1;
	p[5] = 0;

	int NextRow = (NumVertices - 1) * 6;
	//create the other triangles
	for (int idx = 6; idx < *NumIndices; idx++) {
		if (idx % NextRow == 0) {
			p[idx] = (idx / NextRow)*NumVertices;
			p[idx + 1] = p[idx] + NumVertices;
			p[idx + 2] = p[idx + 1] + 1;
			p[idx + 3] = p[idx + 2];
			p[idx + 4] = p[idx] + 1;
			p[idx + 5] = p[idx];

			idx += 5;
		}
		else {
			p[idx] = p[idx - 6] + 1;
		}
	}

}


bool CreateShaders(HWND hMainWnd)
{

	ID3DBlob *pVSBlob = NULL;
	ID3DBlob *pPSBlob = NULL;

	HRESULT hr;

	hr = D3DReadFileToBlob(L".\\SkyboxVertexShader.cso", &pVSBlob); 

	if (hr != S_OK) 
	{
		MessageBox(hMainWnd, L"Problem loading skybox vertex shader.  Check shader file (SkyboxVertexShader.cso) is in the project directory (sub folder of main solution directory) and shader is valid", szAppClassName, MB_OK);

		return false;
	}

	hr = d3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &skyboxVertexShader);

	if (hr != S_OK)
	{
		pVSBlob->Release();

		MessageBox(hMainWnd, L"The skybox vertex shader object cannot be created", szAppClassName, MB_OK);

		return false;
	}

	hr = D3DReadFileToBlob(L".\\SkyboxPixelShader.cso", &pPSBlob); 

	if (hr != S_OK) 
	{

		MessageBox(hMainWnd, L"Problem loading or compiling skybox pixel shader.  Check shader file (SkyboxPixelShader.cso) is in the project directory (sub folder of main solution directory) and shader is valid", szAppClassName, MB_OK);

		return false;
	}

	hr = d3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &skyboxPixelShader);

	pPSBlob->Release();
	if (hr != S_OK) {
		MessageBox(hMainWnd, L"The skybox pixel shader object cannot be created", szAppClassName, MB_OK);

		return false;
	}

	D3D11_INPUT_ELEMENT_DESC skybox_layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,0 }, 
	};

	UINT numElements = 1;

	hr = d3dDevice->CreateInputLayout(skybox_layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &skyboxVertexLayout);

	if (hr != S_OK) { 
		MessageBox(hMainWnd, L"Failed to create skybox input layout", szAppClassName, MB_OK);
		return false;
	}

	pVSBlob->Release();

	hr = D3DReadFileToBlob(L".\\VertexShader.cso", &pVSBlob); 

	if (hr != S_OK)
	{
		MessageBox(hMainWnd, TEXT("Problem loading or compiling vertex shader.  Check shader file (VertexShader.cso) is in the project directory (sub folder of main solution directory) and shader is valid"), szAppClassName, MB_OK);

		return false;
	}

	hr = d3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &vertexShader);

	if (hr != S_OK)
	{
		pVSBlob->Release();

		MessageBox(hMainWnd, TEXT("The vertex shader object cannot be created"), szAppClassName, MB_OK);

		return false;
	}
	hr = D3DReadFileToBlob(L".\\PixelShader.cso", &pPSBlob);

	if (hr != S_OK)
	{

		MessageBox(hMainWnd, TEXT("Problem loading the pixel shader.  Check shader file (PixelShader.cso) is in the project directory (sub folder of main solution directory) and shader is valid"), szAppClassName, MB_OK);

		return false;
	}

	hr = d3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &pixelShader);

	pPSBlob->Release();
	if (hr != S_OK) {
		MessageBox(hMainWnd, TEXT("The pixel shader object cannot be created"), szAppClassName, MB_OK);

		return false;
	}

	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT , 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

	};

	numElements = 2;

	hr = d3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &vertexLayout);

	if (hr != S_OK) {
		MessageBox(hMainWnd, TEXT("Couldn't create input layout"), szAppClassName, MB_OK);

		return false;
	}

	pVSBlob->Release();

	//--------------------------------------------------------------------------------------
	// Textures for terrain and models

	hr = CreateWICTextureFromFile(d3dDevice, d3dContext, L".\\assets\\terrain.jpg", 0, &terrainTexture, 0);

	hr = CreateWICTextureFromFile(d3dDevice, d3dContext, L".\\assets\\model1.jpg", NULL, &modelOne);

	if (hr != S_OK) {
		MessageBox(hMainWnd, TEXT("Unable to load Model 1 texture"), szAppClassName, MB_OK);
		return false;
	}
	hr = CreateWICTextureFromFile(d3dDevice, d3dContext, L".\\assets\\model2.jpg", NULL, &modelTwo);

	if (hr != S_OK) {
		MessageBox(hMainWnd, TEXT("Unable to load Model 2 texture"), szAppClassName, MB_OK);
		return false;
	}
	hr = CreateWICTextureFromFile(d3dDevice, d3dContext, L".\\assets\\model3.jpg", NULL, &modelThree);

	if (hr != S_OK) {
		MessageBox(hMainWnd, TEXT("Unable to load Model 3 texture"), szAppClassName, MB_OK);
		return false;
	}
	hr = CreateWICTextureFromFile(d3dDevice, d3dContext, L".\\assets\\model4.png", NULL, &modelFour);

	if (hr != S_OK) {
		MessageBox(hMainWnd, TEXT("Unable to load Model 4 texture"), szAppClassName, MB_OK);
		return false;
	}
	hr = CreateWICTextureFromFile(d3dDevice, d3dContext, L".\\assets\\model5.jpg", NULL, &modelFive);

	if (hr != S_OK) {
		MessageBox(hMainWnd, TEXT("Unable to load Model 5 texture"), szAppClassName, MB_OK);
		return false;
	}



	
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));

	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = d3dDevice->CreateSamplerState(&sampDesc, &sampler);

	if (hr != S_OK) {
		MessageBox(hMainWnd, TEXT("Unable to create sampler"), szAppClassName, MB_OK);

		return false;
	}

	return true;

}

//Functions for model
bool createmodel1(HWND hMainWnd) 
{

	Assimp::Importer imp; 

	const aiScene *pScene = imp.ReadFile(".\\assets\\model1.obj", 
		aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded);

	if (!pScene) { 
		return false;
	}

	const aiMesh *mesh = pScene->mMeshes[0]; 
	NumVertices = mesh->mNumFaces * 3; 
	theModel *modelOne = new theModel[NumVertices];

	int vertCount = 0;
	for (int faceIdx = 0; faceIdx < mesh->mNumFaces; faceIdx++) {
		const aiFace& face = mesh->mFaces[faceIdx];

		for (int vertIdx = 0; vertIdx < 3; vertIdx++) {
			const aiVector3D *pos = &mesh->mVertices[face.mIndices[vertIdx]];
			const aiVector3D *tex = &mesh->mTextureCoords[0][face.mIndices[vertIdx]];

			modelOne[vertCount] = theModel(pos->x, pos->y, pos->z, tex->x, tex->y); 
			vertCount++;
		}
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT; 
	bd.ByteWidth = sizeof(theModel) * NumVertices;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = modelOne;
	HRESULT hr = d3dDevice->CreateBuffer(&bd, &InitData, &model1);

	if (hr != S_OK) { // Error checking
		MessageBox(hMainWnd, TEXT("Unable to create Model One"), szAppClassName, MB_OK);
		return false;
	}

	delete[] modelOne; 

	return true; 

}

bool createmodel2(HWND hMainWnd)
{

	Assimp::Importer imp2;

	const aiScene *pScene2 = imp2.ReadFile(".\\assets\\model2.obj", aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded);
	if (!pScene2) {
		return false;
	}

	const aiMesh *mesh2 = pScene2->mMeshes[0];
	NumVertices2 = mesh2->mNumFaces * 3;
	theModel *modelTwo = new theModel[NumVertices2];

	int vertCount2 = 0;
	for (int faceIdx2 = 0; faceIdx2 < mesh2->mNumFaces; faceIdx2++) {
		const aiFace& face2 = mesh2->mFaces[faceIdx2];

		for (int vertIdx = 0; vertIdx < 3; vertIdx++) {
			const aiVector3D *pos2 = &mesh2->mVertices[face2.mIndices[vertIdx]];
			const aiVector3D *tex2 = &mesh2->mTextureCoords[0][face2.mIndices[vertIdx]];

			modelTwo[vertCount2] = theModel(pos2->x, pos2->y, pos2->z, tex2->x, tex2->y);
			vertCount2++;
		}
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(theModel) * NumVertices2;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = modelTwo;
	HRESULT hr = d3dDevice->CreateBuffer(&bd, &InitData, &model2);

	if (hr != S_OK) {
		MessageBox(hMainWnd, TEXT("Unable to create Model Two"), szAppClassName, MB_OK);
		return false;
	}

	delete[] modelTwo;

	return true;

}

bool createmodel3(HWND hMainWnd)
{

	Assimp::Importer imp3;

	const aiScene *pScene3 = imp3.ReadFile(".\\assets\\model3.obj", aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded);
	if (!pScene3) {
		return false;
	}

	const aiMesh *mesh3 = pScene3->mMeshes[0];
	NumVertices3 = mesh3->mNumFaces * 3;
	theModel *modelThree = new theModel[NumVertices3];

	int vertCount3 = 0;
	for (int faceIdx3 = 0; faceIdx3 < mesh3->mNumFaces; faceIdx3++) {
		const aiFace& face3 = mesh3->mFaces[faceIdx3];

		for (int vertIdx = 0; vertIdx < 3; vertIdx++) {
			const aiVector3D *pos3 = &mesh3->mVertices[face3.mIndices[vertIdx]];
			const aiVector3D *tex3 = &mesh3->mTextureCoords[0][face3.mIndices[vertIdx]];

			modelThree[vertCount3] = theModel(pos3->x, pos3->y, pos3->z, tex3->x, tex3->y);
			vertCount3++;
		}
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(theModel) * NumVertices3;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = modelThree;
	HRESULT hr = d3dDevice->CreateBuffer(&bd, &InitData, &model3);

	if (hr != S_OK) {
		return false;
	}

	delete[] modelThree;

	return true;
}

bool createmodel4(HWND hMainWnd)
{

	Assimp::Importer imp4;

	const aiScene *pScene4 = imp4.ReadFile(".\\assets\\model4.obj", aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded);

	if (!pScene4) {
		return false;
	}

	const aiMesh *mesh4 = pScene4->mMeshes[0];
	NumVertices4 = mesh4->mNumFaces * 3;
	theModel *modelFour = new theModel[NumVertices4];

	int vertCount4 = 0;
	for (int faceIdx4 = 0; faceIdx4 < mesh4->mNumFaces; faceIdx4++) {
		const aiFace& face4 = mesh4->mFaces[faceIdx4];

		for (int vertIdx = 0; vertIdx < 3; vertIdx++) {
			const aiVector3D *pos4 = &mesh4->mVertices[face4.mIndices[vertIdx]];
			const aiVector3D *tex4 = &mesh4->mTextureCoords[0][face4.mIndices[vertIdx]];

			modelFour[vertCount4] = theModel(pos4->x, pos4->y, pos4->z, tex4->x, tex4->y);
			vertCount4++;
		}
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(theModel) * NumVertices4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = modelFour;
	HRESULT hr = d3dDevice->CreateBuffer(&bd, &InitData, &model4);

	if (hr != S_OK) {
		MessageBox(hMainWnd, TEXT("Unable to create Model Four"), szAppClassName, MB_OK);
		return false;
	}

	delete[] modelFour;

	return true;

}

bool createmodel5(HWND hMainWnd)
{

	Assimp::Importer imp5;

	const aiScene *pScene5 = imp5.ReadFile(".\\assets\\model5.obj", aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded);
	if (!pScene5) {
		return false;
	}

	const aiMesh *mesh5 = pScene5->mMeshes[0];
	NumVertices5 = mesh5->mNumFaces * 3;
	theModel *modelFive = new theModel[NumVertices5];

	int vertCount5 = 0;
	for (int faceIdx5 = 0; faceIdx5 < mesh5->mNumFaces; faceIdx5++) {
		const aiFace& face5 = mesh5->mFaces[faceIdx5];

		for (int vertIdx = 0; vertIdx < 3; vertIdx++) {
			const aiVector3D *pos5 = &mesh5->mVertices[face5.mIndices[vertIdx]];
			const aiVector3D *tex5 = &mesh5->mTextureCoords[0][face5.mIndices[vertIdx]];

			modelFive[vertCount5] = theModel(pos5->x, pos5->y, pos5->z, tex5->x, tex5->y);
			vertCount5++;
		}
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(theModel) * NumVertices5;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = modelFive;
	HRESULT hr = d3dDevice->CreateBuffer(&bd, &InitData, &model5);

	if (hr != S_OK) {
		MessageBox(hMainWnd, TEXT("Unable to create Model Five"), szAppClassName, MB_OK);
		return false;
	}

	delete[] modelFive;

	return true;

}

bool CreateMeshes(HWND hMainWnd) {
	//Define variables to store the terrain vertex and index data


	theTerrain *terrainVertices;
	int *terrainIndices;
	int NumVert;

	//Create the terrain mesh

	CreateTerrainMesh(256, &terrainVertices, &NumVert, &terrainIndices, &NumIndices);
	LoadHeightMap(".\\assets\\terrain.bmp", 21.0f, &terrainVertices);

	//Define the vertex buffer for the terrain
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(theTerrain) * NumVert;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;



	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = terrainVertices;
	HRESULT hr = d3dDevice->CreateBuffer(&bd, &InitData, &terrainVertexBuffer);

	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Failed to create vertex buffer", szAppClassName, NULL);

		return false;
	}

	//Define the index buffer

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(int) *NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = terrainIndices;
	hr = d3dDevice->CreateBuffer(&bd, &InitData, &terrainIndexBuffer);

	



	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Failed to create index buffer", szAppClassName, NULL);

		return false;
	}


	//Free memory allocated to arrays
	delete[] terrainVertices;
	delete[] terrainIndices;


	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = d3dDevice->CreateBuffer(&bd, NULL, &constantBuffer);
	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Failed to create constant buffer", szAppClassName, NULL);

		return false;
	}

	//create fog buffer
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(FogCb);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = d3dDevice->CreateBuffer(&bd, NULL, &fogConstantBuffer);
	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Failed to create fog buffer", szAppClassName, NULL);

		return false;
	}

	return true;
}

void Update() {
	DWORD now = GetTickCount();
	DWORD diff = now - frameTime;

	const float move_step = 20.0f;

	// Store the calculated amount of movement
	// note that there is no rotation variable as this is calculated from mouse movement
	float forwards_backwards = 0.0f;
	float up_down = 0.0f;

	// Calculate the amount of movement forwards and backwards
	if (IsKeyDown('W')) {
		forwards_backwards = move_step * (diff / 1000.0f);
	}

	if (IsKeyDown('S')) {
		forwards_backwards = -move_step * (diff / 1000.0f);
	}

	// Calculate the amount of movement forwards and backwards
	if (IsKeyDown('Q')) {
		up_down = move_step * (diff / 1000.0f);
	}

	if (IsKeyDown('E')) {
		up_down = -move_step * (diff / 1000.0f);
	}

	// Calculate the change mouse X pos
	int deltaX = clientMidX - curMouseXPos;

	// Update the rotation 
	camRotY -= deltaX * rotFactorY;

	// Reset cursor position to middle of window
	SetCursorPos(winLeft + winMidX, winTop + winMidY);

	XMVECTOR oldCamTarget = camTarget;
	float oldCamX = camXPos, oldCamY = camYPos, oldCamZ = camZPos;

	// Calculate distance to move camera with respect to movement on Z axis and rotation
	XMMATRIX camMove = XMMatrixTranslation(0.0f, up_down, forwards_backwards) *
		XMMatrixRotationRollPitchYaw(camRotX, camRotY, 0.0f);

	// Obtain the translation from the move matrix
	XMVECTOR scale, rot, trans;
	XMMatrixDecompose(&scale, &rot, &trans, camMove);

	// Update the camera position X, Y and Z
	camXPos += XMVectorGetX(trans);
	camYPos += XMVectorGetY(trans);
	camZPos += XMVectorGetZ(trans);

	XMVECTOR camPos = XMVectorSet(camXPos, camYPos, camZPos, 0.0f);

	// Calculate the relative distance from the camera to look at with respect to distance and rotation
	XMMATRIX camDist = XMMatrixTranslation(0.0f, 0.0f, 10.0f) *
		XMMatrixRotationRollPitchYaw(camRotX, camRotY, 0.0f);

	// Obtain the translation and calculate target
	XMMatrixDecompose(&scale, &rot, &trans, camDist);

	// Calculate a new target to look at with respect to the camera position
	camTarget = XMVectorSet(camXPos + XMVectorGetX(trans),
		camYPos + XMVectorGetY(trans),
		camZPos + XMVectorGetZ(trans), 0.0f);

	matView = XMMatrixLookAtLH( camPos, camTarget, camUp );

	frameTime = now;
}
void Draw()
{

	if (d3dContext == NULL)
	{

		return;

	}

	float colour[] = { 0.392156f, 0.584313f, 0.929411f, 1.0f };
	d3dContext->ClearRenderTargetView(backBuffer, colour);
	d3dContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	ConstantBuffer cb;
	matWorld = XMMatrixTranslation(camXPos, camYPos, camZPos);
	cb.mWorld = XMMatrixTranspose(matWorld);
	cb.mView = XMMatrixTranspose(matView);
	cb.mProjection = XMMatrixTranspose(matProjection);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	d3dContext->VSSetConstantBuffers(0, 1, &constantBuffer);
	d3dContext->VSSetShader(skyboxVertexShader, NULL, 0);
	d3dContext->PSSetShader(skyboxPixelShader, NULL, 0);
	d3dContext->PSSetSamplers(0, 1, &sampler);
	d3dContext->PSSetShaderResources(1, 1, &cubeMap);
	d3dContext->IASetInputLayout(skyboxVertexLayout);
	UINT stride = sizeof(XMFLOAT3);
	UINT offset = 0;
	d3dContext->IASetVertexBuffers(0, 1, &skyboxVertexBuffer, &stride, &offset);
	d3dContext->OMSetDepthStencilState(DSDepthOff, 0);
	d3dContext->RSSetState(rsCullingOff);
	d3dContext->Draw(ARRAYSIZE(skybox), 0);
	d3dContext->RSSetState(rsCullingOn);
	d3dContext->OMSetDepthStencilState(DSDepthOn, 0);
	d3dContext->IASetInputLayout(vertexLayout);
	d3dContext->VSSetShader(vertexShader, NULL, 0);
	d3dContext->PSSetShader(pixelShader, NULL, 0);

	//Terrain
	matWorld = XMMatrixScaling(50.0f, 0.2f, 100.0f) * XMMatrixIdentity();

	cb.mWorld = XMMatrixTranspose(matWorld);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);

	stride = sizeof(theTerrain);
	offset = 0;
	d3dContext->IASetVertexBuffers(0, 1, &terrainVertexBuffer, &stride, &offset);

	d3dContext->IASetIndexBuffer(terrainIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	d3dContext->PSSetShaderResources(1, 1, &terrainTexture);

	d3dContext->DrawIndexed(NumIndices, 0, 0);

	//MODELS--------------------------------------
	// Model One
	matWorld = XMMatrixScaling(0.1f, 0.1f, 0.1f)  *XMMatrixRotationY(-125.0f) * XMMatrixTranslation(-25.0f, 1.0f, 15.0f);
	cb.mWorld = XMMatrixTranspose(matWorld);
	cb.mView = XMMatrixTranspose(matView);
	cb.mProjection = XMMatrixTranspose(matProjection);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);

	d3dContext->IASetVertexBuffers(0, 1, &model1, &stride, &offset);
	d3dContext->PSSetShaderResources(1, 1, &modelOne);
	d3dContext->Draw(NumVertices, 0);


	// Model Two
	matWorld = XMMatrixScaling(5.0f, 5.0f, 5.0f) * XMMatrixTranslation(5.0f, -2.0f, 18.0f);
	cb.mWorld = XMMatrixTranspose(matWorld);
	cb.mView = XMMatrixTranspose(matView);
	cb.mProjection = XMMatrixTranspose(matProjection);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);

	d3dContext->IASetVertexBuffers(0, 1, &model2, &stride, &offset);
	d3dContext->PSSetShaderResources(1, 1, &modelTwo);
	d3dContext->Draw(NumVertices2, 0);


	// Model Three
	matWorld = XMMatrixScaling(0.2f, 0.2f, 0.2f) * XMMatrixTranslation(-10.0f, 3.3f, 15.0f);
	cb.mWorld = XMMatrixTranspose(matWorld);
	cb.mView = XMMatrixTranspose(matView);
	cb.mProjection = XMMatrixTranspose(matProjection);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);

	d3dContext->IASetVertexBuffers(0, 1, &model3, &stride, &offset);
	d3dContext->PSSetShaderResources(1, 1, &modelThree);
	d3dContext->Draw(NumVertices3, 0);

	// Model Four
	matWorld = XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixRotationY(-125.0f) * XMMatrixTranslation(15.0f, 1.5f, 1.0f);
	cb.mWorld = XMMatrixTranspose(matWorld);
	cb.mView = XMMatrixTranspose(matView);
	cb.mProjection = XMMatrixTranspose(matProjection);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);

	d3dContext->IASetVertexBuffers(0, 1, &model4, &stride, &offset);
	d3dContext->PSSetShaderResources(1, 1, &modelFour);
	d3dContext->Draw(NumVertices4, 0);

	// Model Five
	matWorld = XMMatrixScaling(2.0f, 2.0f, 2.0f)   * XMMatrixTranslation(15.0f, 3.1f, 25.0f);
	cb.mWorld = XMMatrixTranspose(matWorld);
	cb.mView = XMMatrixTranspose(matView);
	cb.mProjection = XMMatrixTranspose(matProjection);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);

	d3dContext->IASetVertexBuffers(0, 1, &model5, &stride, &offset);
	d3dContext->PSSetShaderResources(1, 1, &modelFive);
	d3dContext->Draw(NumVertices5, 0);

	//Fog
	FogCb fogParams;
	fogParams.eyePos = XMFLOAT3(camXPos, camYPos, camZPos);
	fogParams.fogColour = XMFLOAT4(0.7f, 0.5f, 0.5f, 1.0f);
	fogParams.fogStart = 10.0f;
	fogParams.fogRange = 100.0f;
	d3dContext->UpdateSubresource(fogConstantBuffer, 0, NULL, &fogParams, 0, 0);
	d3dContext->PSSetConstantBuffers(0, 1, &fogConstantBuffer);



	swapChain->Present(0, 0);

}

void ShutdownDirectX()
{

	if (cubeMap) {
		cubeMap->Release();
	}

	if (rsCullingOn) {
		rsCullingOn->Release();
	}

	if (rsCullingOff) {
		rsCullingOff->Release();
	}

	if (DSDepthOn) {
		DSDepthOn->Release();
	}

	if (DSDepthOff) {
		DSDepthOff->Release();
	}

	if (skyboxVertexBuffer) {
		skyboxVertexBuffer->Release();
	}

	if (skyboxVertexLayout) {
		skyboxVertexLayout->Release();
	}

	if (skyboxPixelShader) {
		skyboxPixelShader->Release();
	}

	if (skyboxVertexShader) {
		skyboxVertexShader->Release();
	}

	if (vertexBuffer != NULL) {
		vertexBuffer->Release();
	}



	if (vertexShader != NULL) {
		vertexShader->Release();
	}


	if (pixelShader != NULL) {
		pixelShader->Release();
	}



	if (vertexLayout != NULL) {
		vertexLayout->Release();
	}


	if (backBuffer) {
		backBuffer->Release();
	}

	if (swapChain) {
		swapChain->Release();
	}

	if (d3dContext) {
		d3dContext->Release();
	}

	if (d3dDevice) {
		d3dDevice->Release();
	}


}

bool CreateCullingRasterizersOnDepthTestStates(HWND hMainWnd) {
	
	D3D11_RASTERIZER_DESC rasDesc;
	ZeroMemory(&rasDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasDesc.FillMode = D3D11_FILL_SOLID;
	rasDesc.CullMode = D3D11_CULL_NONE;
	rasDesc.DepthClipEnable = true;

	
	HRESULT result = d3dDevice->CreateRasterizerState(&rasDesc, &rsCullingOff);
	if (result != S_OK) {
		MessageBox(hMainWnd, TEXT("Failed to create rasterizer state"), szAppClassName, NULL);
	}

	
	ZeroMemory(&rasDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasDesc.FillMode = D3D11_FILL_SOLID;
	rasDesc.CullMode = D3D11_CULL_BACK;
	result = d3dDevice->CreateRasterizerState(&rasDesc, &rsCullingOn);
	if (result != S_OK) {
		MessageBox(hMainWnd, TEXT("Failed to create rasterizer state"), szAppClassName, NULL);
	}

	
	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = false;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

	d3dDevice->CreateDepthStencilState(&dssDesc, &DSDepthOff);

	
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS;

	d3dDevice->CreateDepthStencilState(&dssDesc, &DSDepthOn);

	return true;
}	

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR szCmdLineArgs, int nInitialWinShowState)

{

	HWND hMainWnd;
	MSG msg = { 0 };

	WNDCLASS wndclass;
	wchar_t fps[32];

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WinProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppClassName;


	if (!RegisterClass(&wndclass)) {
		MessageBox(NULL, TEXT("Unable to register class for application"), szAppClassName, 0);

		return 0;
	}

	hMainWnd = CreateWindow(szAppClassName,
		TEXT("Scene"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!hMainWnd) {
		MessageBox(NULL, TEXT("Unable able to create the application's main window"), szAppClassName, 0);

		return 0;
	}


	ShowWindow(hMainWnd, nInitialWinShowState);

	
	if (!InitialiseDirectX(hMainWnd, hInstance)) {
		MessageBox(NULL, TEXT("Failed to initialise DirectX"), szAppClassName, 0);

		return 0;
	}

	
	if (!CreateShaders(hMainWnd)) {

		return 0;
	}

	if (!CreateMeshes(hMainWnd)) {
		
		return 0;
	}


	

	if (!CreateCubemapSkybox(L".\\assets\\up.jpg", L".\\assets\\down.jpg", L".\\assets\\right.jpg",
		L".\\assets\\left.jpg", L".\\assets\\front.jpg", L".\\assets\\back.jpg", 512)) {
		return 0;
	}
	if (!createmodel1(hMainWnd))
	{
		return 0;
	}

	if (!createmodel2(hMainWnd))
	{
		return 0;
	}


	if (!createmodel3(hMainWnd))
	{
		return 0;
	}


	if (!createmodel4(hMainWnd))
	{
		return 0;
	}

	if (!createmodel5(hMainWnd))
	{
		return 0;
	}


	CreateCullingRasterizersOnDepthTestStates(hMainWnd);

	DWORD current = GetTickCount();
	frameTime = GetTickCount();
	int count = 0;
	InitialiseKeyboardHandler();

	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			

			Update();


			

			Draw();



			count++;

			DWORD now = GetTickCount();
			if (now - current > 1000) {
				swprintf(fps, 32, L"FPS = %d", count);

				SetWindowText(hMainWnd, fps);

				count = 0;

				current = now;
			}

		}
	}

	

	ShutdownDirectX();

	return 0;

}

LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		

	case WM_KEYDOWN:
		ProcessKeyDown(wParam);

		return 0;

	case WM_KEYUP:
		ProcessKeyUp(wParam);
		return 0;

	case WM_MOUSEMOVE:
		curMouseXPos = LOWORD(lParam);
		curMouseYPos = HIWORD(lParam);

		return 0;


	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}



