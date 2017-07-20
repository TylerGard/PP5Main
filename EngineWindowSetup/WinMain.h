#pragma once

#include <d3d11.h>
#include <D3DX11.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

struct pipeline_state_t {
	ID3D11InputLayout *input_layout;
	ID3D11VertexShader *vertex_shader;
	ID3D11PixelShader *pixel_shader;
	ID3D11RenderTargetView *render_target;
	ID3D11Texture2D *depthStencilBuffer;
	ID3D11DepthStencilState *depthStencilState;
	ID3D11DepthStencilView *depthStencilView;
	ID3D11RasterizerState *rasterState;
	ID3D11RasterizerState *debugRasterState;
	ID3D11Buffer* buffer;
	ID3D11Buffer* indexBuffer;
	ID3D11Buffer * boneVertexBuffer;
	ID3D11Buffer * boneIndexBuffer;

	ID3D11Buffer * debugBuffer = nullptr;
	size_t debugBufferSize = 0;

	ID3D11Buffer * groundBuffer = nullptr;
	
};

struct ModelViewProjectionConstantBuffer {
	XMFLOAT4X4 model;
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
};
struct VertexPosColor {
	XMFLOAT3 Position;
	XMFLOAT4 Color;
};

class WindowClass {
	std::vector<VertexPosColor> linesVector;
	HWND hWND;
	std::vector<pipeline_state_t *> pipelineStates;
	//pipeline_state_t *pipelineState;
	ID3D11Device *device;
	ID3D11DeviceContext *deviceContext;
	IDXGISwapChain *swapChain = nullptr;
	ID3D11RenderTargetView *renderTargetView;
	ID3D11Buffer *constantBuffer;
	ModelViewProjectionConstantBuffer constantBufferData;
	ID3D11Buffer *lineBuffer;
	D3D_DRIVER_TYPE driverType;
	D3D_FEATURE_LEVEL featureLevel;
	D3D11_VIEWPORT viewport;
	bool debugWires;

public:
	void start(HWND wind, int width, int height);
	WindowClass();
	~WindowClass();
	void Render();
	void debugSwitch();
	void UpdateMouse(float delta, float x, float y);
	//std::vector<VertexPosColor >vertices;
	//std::vector<int> index;
	//std::vector<VertexPosColor> boneVertices;

	std::vector<std::vector<VertexPosColor> >vertices;
	std::vector<std::vector<int>> index;
	std::vector<std::vector<VertexPosColor>> boneVertices;
	void addToBuffer(VertexPosColor VPC);
	void clearBuffer();
	
	void addLine(VertexPosColor vertOne, VertexPosColor vertTwo);

	void clearDebugBuffer();
};