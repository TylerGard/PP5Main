#include "stdafx.h"
#include "WinMain.h"

using namespace DirectX;

WindowClass::WindowClass()
{
	
}

void readFile(char *name, char *&data, long &size)
{
	FILE* file;
	fopen_s(&file, name, "rb");
	if (!file) {
		size = 0;
		data = nullptr;
		printf("%s does not exist", name);
		return;
	}
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	rewind(file);
	data = (char*)malloc(size);
	if (!*data) {
		fclose(file);
		size = 0;
		data = nullptr;
		printf("%s has no data or can't be read", name);
		return;
	}
	if (!fread(data, size, 1, file)) {
		fclose(file);
		free(data);
		size = 0;
		data = nullptr;
		printf("Problem reading %s", name);
		return;
	}
	fclose(file);
}

void WindowClass::start(HWND window, int width, int height) {
	hWND = window;
	UINT errorcheck = 0;

#ifdef _DEBUG
	errorcheck |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	//feature array configuring
	D3D_FEATURE_LEVEL featureArray[] = {
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	UINT featureArraySize = ARRAYSIZE(featureArray);

	//swapChainDesc configuring
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = window;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT Result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, errorcheck, featureArray, featureArraySize, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, &featureLevel, &deviceContext);

	//texture
	ID3D11Texture2D *texture = 0;
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&texture);
	device->CreateRenderTargetView(texture, nullptr, &renderTargetView);
	texture->Release();

	//deviceContext
	deviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);

	//viewport
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	deviceContext->RSSetViewports(1, &viewport);

	//pipelineState
	pipelineState = new pipeline_state_t();

	//debug stuff
	D3D11_RASTERIZER_DESC rastDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());

	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_BACK;
	device->CreateRasterizerState(&rastDesc, &(pipelineState->rasterState));


	rastDesc.FillMode = D3D11_FILL_WIREFRAME;
	rastDesc.CullMode = D3D11_CULL_NONE;
	device->CreateRasterizerState(&rastDesc, &(pipelineState->debugRasterState));

	//pixel shader
	char *pixelData;
	long pixelSize;
	readFile("Trivial_PS.cso", pixelData, pixelSize);
	device->CreatePixelShader(pixelData, pixelSize, NULL, &pipelineState->pixel_shader);
	free(pixelData);

	//vertex shader
	char *vertexData;
	long vertexSize;
	readFile("Trivial_VS.cso", vertexData, vertexSize);
	device->CreateVertexShader(vertexData, vertexSize, NULL, &pipelineState->vertex_shader);

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	device->CreateInputLayout(inputElementDesc, 2, vertexData, vertexSize, &pipelineState->input_layout);
	free(vertexData);

	deviceContext->IASetInputLayout(pipelineState->input_layout);
	//triangle
	/*VertexPosColor vertexes[] = {
		{ XMFLOAT3(0.0f, 0.5f, 0.0f),XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.45f, -0.5, 0.0f),XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.45f, -0.5f, 0.0f),XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }
	};
*/
	
	ID3D11Buffer * newVertexBuffer = nullptr;
	ID3D11Buffer * newIndexBuffer = nullptr;
	
	//for (int i = 0; i < pipelineState->buffer.size(); i++) {
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = sizeof(VertexPosColor) * vertices.size();
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		device->CreateBuffer(&bufferDesc, NULL, &(pipelineState->buffer));


		//D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		//deviceContext->Map(pipelineState->buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
		//memcpy(mappedSubresource.pData, vertexes, sizeof(vertexes));
		//deviceContext->Unmap(pipelineState->buffer, NULL);

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = &vertices[0];
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;

		CD3D11_BUFFER_DESC bufferDescVector(sizeof(VertexPosColor)*vertices.size(), D3D11_BIND_VERTEX_BUFFER);
		device->CreateBuffer(&bufferDescVector, &vertexBufferData, &pipelineState->buffer);
		D3D11_BUFFER_DESC bufferDescIndices;
		ZeroMemory(&bufferDescIndices, sizeof(bufferDescIndices));
		bufferDescIndices.Usage = D3D11_USAGE_DYNAMIC;
		bufferDescIndices.ByteWidth = sizeof(int)*index.size();
		bufferDescIndices.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDescIndices.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		device->CreateBuffer(&bufferDescIndices, NULL, &pipelineState->indexBuffer);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = &index[0];
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC bufferDescriptionIndex(sizeof(int)*index.size(), D3D11_BIND_INDEX_BUFFER);
		device->CreateBuffer(&bufferDescriptionIndex, &indexBufferData, &pipelineState->indexBuffer);


		//Start Bone////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		D3D11_BUFFER_DESC boneBufferDesc;
		ZeroMemory(&boneBufferDesc, sizeof(boneBufferDesc));
		boneBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		boneBufferDesc.ByteWidth = sizeof(VertexPosColor) * boneVertices.size();
		boneBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		boneBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		device->CreateBuffer(&boneBufferDesc, NULL, &(pipelineState->boneVertexBuffer));


		//D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		//deviceContext->Map(pipelineState->buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
		//memcpy(mappedSubresource.pData, vertexes, sizeof(vertexes));
		//deviceContext->Unmap(pipelineState->buffer, NULL);

		D3D11_SUBRESOURCE_DATA boneVertexBufferData = { 0 };
		boneVertexBufferData.pSysMem = &boneVertices[0];
		boneVertexBufferData.SysMemPitch = 0;
		boneVertexBufferData.SysMemSlicePitch = 0;

		CD3D11_BUFFER_DESC boneBufferDescVector(sizeof(VertexPosColor)*boneVertices.size(), D3D11_BIND_VERTEX_BUFFER);
		device->CreateBuffer(&boneBufferDescVector, &boneVertexBufferData, &pipelineState->boneVertexBuffer);
		D3D11_BUFFER_DESC boneBufferDescIndices;
		ZeroMemory(&boneBufferDescIndices, sizeof(boneBufferDescIndices));
		boneBufferDescIndices.Usage = D3D11_USAGE_DYNAMIC;
		boneBufferDescIndices.ByteWidth = sizeof(int)*index.size();
		boneBufferDescIndices.BindFlags = D3D11_BIND_INDEX_BUFFER;
		boneBufferDescIndices.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		device->CreateBuffer(&boneBufferDescIndices, NULL, &pipelineState->boneIndexBuffer);

		D3D11_SUBRESOURCE_DATA boneIndexBufferData = { 0 };
		boneIndexBufferData.pSysMem = &index[0];
		boneIndexBufferData.SysMemPitch = 0;
		boneIndexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC boneBufferDescriptionIndex(sizeof(int)*index.size(), D3D11_BIND_INDEX_BUFFER);
		device->CreateBuffer(&boneBufferDescriptionIndex, &boneIndexBufferData, &pipelineState->boneIndexBuffer);
		//End Bone////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	}
	//lines

	/*VertexPosColor lines[] =
	{
		{ XMFLOAT3(0.0f, 0.0f, 0.0f),XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.0f, 0.0f, 0.0f),XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.0f, 0.0f, 0.0f),XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }
	};

	ZeroMemory(&bufferDesc, sizeof(bufferDesc));

	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(VertexPosColor) * 6;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	device->CreateBuffer(&bufferDesc, NULL, &lineBuffer);

	deviceContext->Map(lineBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, lines, sizeof(lines));
	deviceContext->Unmap(lineBuffer, NULL);*/

	//camera

	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	device->CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer);

	static const XMVECTORF32 eyePos = { 10.5f, 10.5f, -2.f, 0.f };
	static const XMVECTORF32 focusPos = { .0f, -.1f, 0.f, 0.f };
	static const XMVECTORF32 upDir = { .0f, 1.f, 0.f, 0.f };
	XMMATRIX focus = XMMatrixLookAtLH(eyePos, focusPos, upDir);
	XMStoreFloat4x4(&constantBufferData.view, XMMatrixTranspose(focus));
	XMStoreFloat4x4(&constantBufferData.projection, XMMatrixTranspose(XMMatrixPerspectiveFovLH(70.0f * XM_PI / 180.0f, float(width) / height, 0.01f, 100.0f)));
	XMStoreFloat4x4(&constantBufferData.model, XMMatrixTranspose(XMMatrixIdentity()));

	//depth

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D *texture2D;
	HRESULT result = device->CreateTexture2D(&textureDesc, NULL, &texture2D);

	if (result != S_OK)
		exit(1);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = textureDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

	result = device->CreateDepthStencilView(texture2D, &depthStencilViewDesc, &(pipelineState->depthStencilView));
	texture2D->Release();

	if (result != S_OK)
		exit(2);

	deviceContext->OMSetRenderTargets(1, &renderTargetView, pipelineState->depthStencilView);
}

#define ReleaseIfExists(x) if(x) x->Release()

WindowClass::~WindowClass() {
	//take care of the pipelineState first
//	for (int i = 0; i < pipelineState->buffer.size(); i++) {
		ReleaseIfExists(pipelineState->buffer);
//	}
	ReleaseIfExists(pipelineState->depthStencilBuffer);
	ReleaseIfExists(pipelineState->depthStencilState);
	ReleaseIfExists(pipelineState->depthStencilView);
	ReleaseIfExists(pipelineState->input_layout);
	ReleaseIfExists(pipelineState->rasterState);
	ReleaseIfExists(pipelineState->debugRasterState);
	ReleaseIfExists(pipelineState->render_target);
	ReleaseIfExists(pipelineState->pixel_shader);
	ReleaseIfExists(pipelineState->vertex_shader);

	delete pipelineState;

	//now work with the rest
	ReleaseIfExists(lineBuffer);
	ReleaseIfExists(constantBuffer);
	ReleaseIfExists(renderTargetView);
	ReleaseIfExists(swapChain);
	ReleaseIfExists(deviceContext);
	ReleaseIfExists(device);
}
#undef ReleaseIfExists

void WindowClass::Render() {
	if (GetActiveWindow() != hWND)
		return;

	if (GetAsyncKeyState(VK_SPACE))
	{
		XMMATRIX model = XMLoadFloat4x4(&constantBufferData.model);
		XMMATRIX trans = XMMatrixTranslation(
			(float)(rand()) / RAND_MAX * 2.0f - 1.0f,
			(float)(rand()) / RAND_MAX * 2.0f - 1.0f,
			(float)(rand()) / RAND_MAX * 2.0f - 1.0f
		);

		XMMATRIX rotX;
		XMMATRIX rotY;
		XMMATRIX rotZ;
		rotX = XMMatrixRotationX((float)(rand()) / RAND_MAX * 2.0f * XM_PI);
		rotY = XMMatrixRotationY((float)(rand()) / RAND_MAX * 2.0f * XM_PI);
		rotZ = XMMatrixRotationZ((float)(rand()) / RAND_MAX * 2.0f * XM_PI);
		model = rotX * rotY * rotZ * trans;
		XMStoreFloat4x4(&constantBufferData.model, XMMatrixTranspose(model));
	}
	
	if (debugWires)
		deviceContext->RSSetState(pipelineState->debugRasterState);
	else
		deviceContext->RSSetState(pipelineState->rasterState);

	deviceContext->ClearDepthStencilView(pipelineState->depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	deviceContext->ClearRenderTargetView(renderTargetView, DirectX::Colors::Black);
	deviceContext->PSSetShader(pipelineState->pixel_shader, NULL, 0);
	deviceContext->VSSetShader(pipelineState->vertex_shader, NULL, 0);
	deviceContext->UpdateSubresource(constantBuffer, 0, NULL, &constantBufferData, 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);

	UINT stride = sizeof(VertexPosColor);
	UINT offset = 0;
//	for (int i = 0; i < pipelineState->buffer.size(); i++) {
		/*deviceContext->IASetVertexBuffers(0, 1, &pipelineState->buffer, &stride, &offset);*/
		deviceContext->IASetIndexBuffer(pipelineState->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		/*deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deviceContext->DrawIndexed(index.size(), 0, 0);*/

		//for(int i = 0; i < )
		 stride = sizeof(VertexPosColor);
		 offset = 0;
		deviceContext->IASetVertexBuffers(0, 1, &pipelineState->boneVertexBuffer, &stride, &offset);
		//deviceContext->IASetIndexBuffer(pipelineState->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		deviceContext->DrawIndexed(boneVertices.size(), 0, 0);
//	}
	/*deviceContext->UpdateSubresource(constantBuffer, 0, NULL, &constantBufferData, 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
	deviceContext->IASetVertexBuffers(0, 1, &lineBuffer, &stride, &offset);
	deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
	deviceContext->Draw(6, 0);*/

	swapChain->Present(0, 0);
}

float roty = 0;
float rotx = 0;
float posx = 0;
float posy = 0;
float posz = 0;
void WindowClass::UpdateMouse(float deltaTime, float x, float y)
{
	const float scaler = 0.0005f;
	roty -= x;
	rotx -= y;
	if (rotx >= XM_PIDIV2)
		rotx = XM_PIDIV2 - 0.001;
	if (rotx <= -XM_PIDIV2)
		rotx = -XM_PIDIV2 + 0.001;
	if (GetAsyncKeyState('W')) {
		posx += cos(roty)*cos(rotx)*deltaTime*scaler;
		posz += sin(roty)*cos(rotx)*deltaTime*scaler;
		posy += sin(rotx)*deltaTime*scaler;
	}
	if (GetAsyncKeyState('A')) {
		posz += cos(roty)*deltaTime*scaler;
		posx -= sin(roty)*deltaTime*scaler;
	}
	if (GetAsyncKeyState('S')) {
		posx -= cos(roty)*cos(rotx)*deltaTime*scaler;
		posz -= sin(roty)*cos(rotx)*deltaTime*scaler;
		posy -= sin(rotx)*deltaTime*scaler;
	}
	if (GetAsyncKeyState('D')) {
		posz -= cos(roty)*deltaTime*scaler;
		posx += sin(roty)*deltaTime*scaler;
	}
	printf("%f, %f, %f\n", posx, posy, posz);
	if (GetAsyncKeyState(VK_SPACE)) posy += deltaTime*scaler;
	if (GetAsyncKeyState(VK_LSHIFT)) posy -= deltaTime*scaler;
	const XMVECTORF32 EyePosition = { posx, posy, posz, 0.f };
	const XMVECTORF32 FocusPosition = { cos(roty)*cos(rotx) + posx, sin(rotx) + posy, sin(roty)*cos(rotx) + posz, 0.f };
	const XMVECTORF32 UpDirection = { .0f, 1.f, 0.f, 0.f };
	XMMATRIX LookAt = XMMatrixLookAtLH(EyePosition, FocusPosition, UpDirection);
	XMStoreFloat4x4(&constantBufferData.view, XMMatrixTranspose(LookAt));
}

void WindowClass::debugSwitch()
{
	debugWires = !debugWires;
}

void WindowClass::addToBuffer(VertexPosColor VPC) {
	boneVertices.push_back(VPC);

	
}

void WindowClass::clearBuffer() {
	boneVertices.clear();
}