#pragma once

struct SimpleVertex
{
	DirectX::XMFLOAT3 Position;
};

//Vertex struct for colors. We do not handle textures.
struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
};