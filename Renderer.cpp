#include "pch.h"
#include "Renderer.h"
#include "DXCore.h"
#include "Window.h"
#include "RenderCommand.h"
#include "Camera.h"
#include "MemoryManager.h"
#define USE_PIX
#include "pix3.h"

void Renderer::Initialize() noexcept
{
	CreateDepthBuffer();
	CreateRootSignature();
	CreatePipelineStateObject();
	CreateViewportAndScissorRect();

	auto pCommandList = DXCore::GetCommandList();

	HR(pCommandList->Close());
	RenderCommand::Flush();
	HR(DXCore::GetCommandAllocators()[0]->Reset());
	HR(pCommandList->Reset(DXCore::GetCommandAllocators()[0].Get(), nullptr));
}

void Renderer::Begin(Camera* const pCamera, D3D12_GPU_VIRTUAL_ADDRESS accelerationStructure) noexcept
{
	auto pCommandAllocator = DXCore::GetCommandAllocators()[m_FrameIndex];
	auto pCommandList = DXCore::GetCommandList();
	auto pBackBuffer = Window::Get().GetBackBuffers()[m_CurrentBackBufferIndex];

	auto& pBackBufferRTVDescHeap = Window::Get().GetBackBufferRTVHeap();
	auto backBufferDescriptorHandle = pBackBufferRTVDescHeap->GetCPUStartHandle();
	backBufferDescriptorHandle.ptr += m_CurrentBackBufferIndex * pBackBufferRTVDescHeap->GetDescriptorTypeSize();
	auto depthBufferDSVHandle = m_pDSVDescriptorHeap->GetCPUStartHandle();

	PIXBeginEvent(DXCore::GetCommandList().Get(), 200, "Renderer::Begin");

	//Clear current back buffer & depth buffer:
	RenderCommand::TransitionResource(pBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	
	RenderCommand::ClearRenderTarget(backBufferDescriptorHandle, DirectX::Colors::Black);
	RenderCommand::ClearDepth(depthBufferDSVHandle, 1.0f);

	//Set back buffer RTV and depth buffer DSV:
	STDCALL(pCommandList->OMSetRenderTargets(1u, &backBufferDescriptorHandle, false, &depthBufferDSVHandle));

	//PSO and Root sig:
	STDCALL(pCommandList->SetPipelineState(m_pPSO.Get()));
	STDCALL(pCommandList->SetGraphicsRootSignature(m_pRootSignature.Get()));
	STDCALL(pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

	STDCALL(pCommandList->RSSetViewports(1u, &m_ViewPort));
	STDCALL(pCommandList->RSSetScissorRects(1u, &m_ScissorRect));

	auto pDescriptorHeap = MemoryManager::Get().GetActiveSRVCBVUAVDescriptorHeap();
	STDCALL(pCommandList->SetDescriptorHeaps(1u, pDescriptorHeap->GetInterface().GetAddressOf()));

	static VP vpMatrixCBuffer;
	auto vpMatrix = DirectX::XMLoadFloat4x4(&(pCamera->GetVPMatrix()));
	vpMatrix = DirectX::XMMatrixTranspose(vpMatrix);
	DirectX::XMStoreFloat4x4(&vpMatrixCBuffer.VPMatrix, vpMatrix);
	STDCALL(pCommandList->SetGraphicsRoot32BitConstants(3u, 4*4, &vpMatrixCBuffer, 0u));

	static InverseVP vpInverseCBuffer;
	auto vpInverse = DirectX::XMLoadFloat4x4(&(pCamera->GetVPMatrix()));
	DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(vpInverse);
	vpInverse = DirectX::XMMatrixInverse(&det, vpInverse);
	vpInverse = DirectX::XMMatrixTranspose(vpInverse);
	DirectX::XMStoreFloat4x4(&vpInverseCBuffer.InverseVPMatrix, vpInverse);
	STDCALL(pCommandList->SetGraphicsRoot32BitConstants(5u, 4 * 4, &vpInverseCBuffer, 0u));

	DirectX::XMFLOAT3 cameraFloat3 = pCamera->GetPosition();
	DirectX::XMFLOAT4 cameraFloat4 = DirectX::XMFLOAT4(cameraFloat3.x, cameraFloat3.y, cameraFloat3.z, (float)pCamera->GetRayTraceBool());
	auto cameraPos = DirectX::XMLoadFloat4(&(cameraFloat4));
	STDCALL(pCommandList->SetGraphicsRoot32BitConstants(7u, 4, &cameraPos, 0u));

	//Raytracing accelerationstructure.
	STDCALL(pCommandList->SetGraphicsRootShaderResourceView(4u, accelerationStructure));
	PIXEndEvent(DXCore::GetCommandList().Get());
}

void Renderer::Submit(const std::unordered_map<std::string, std::vector<std::shared_ptr<VertexObject>>>& vertexObjects) noexcept
{
	PIXBeginEvent(DXCore::GetCommandList().Get(), 300, "Renderer::Submit");
	static float speed = 1.0f;
	auto pCommandList = DXCore::GetCommandList();
	for (auto& modelInstances : vertexObjects)
	{
		for (auto& object : modelInstances.second)
		{
			auto objectColor = DirectX::XMLoadFloat4(&(object->GetColor()));
			STDCALL(pCommandList->SetGraphicsRoot32BitConstants(6u, 4, &objectColor, 0u));

			const std::vector<std::unique_ptr<Mesh>>& objectMeshes = object->GetModel()->GetMeshes();
			for (uint32_t i{ 0u }; i < objectMeshes.size(); i++)
			{
				auto& cbv = object->GetTransformConstantBufferView();
				auto gpuHandle = cbv.GpuHandles[m_FrameIndex];

				STDCALL(pCommandList->SetGraphicsRootDescriptorTable(0, gpuHandle));
				STDCALL(pCommandList->SetGraphicsRootShaderResourceView(1u, objectMeshes[i]->GetVertexBufferGPUAddress()));
				STDCALL(pCommandList->SetGraphicsRootShaderResourceView(2u, objectMeshes[i]->GetIndexBufferGPUAddress()));
				STDCALL(pCommandList->DrawInstanced(objectMeshes[i]->GetIndexCount(), 1u, 0u, 0u));
			}
		}
	}
	PIXEndEvent(DXCore::GetCommandList().Get());
}

void Renderer::End() noexcept
{
	auto pCommandList = DXCore::GetCommandList();
	auto pBackBuffer = Window::Get().GetBackBuffers()[m_CurrentBackBufferIndex];
	//Present:
	{
		RenderCommand::TransitionResource(pBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		HR(pCommandList->Close());

		ID3D12CommandList* commandLists[] = { pCommandList.Get() };
		STDCALL(DXCore::GetCommandQueue()->ExecuteCommandLists(ARRAYSIZE(commandLists), commandLists));

		Window::Get().Present();
		m_CurrentBackBufferIndex = Window::Get().GetCurrentBackbufferIndex();

		WaitAndSync();
	}
}


void Renderer::OnShutDown() noexcept
{
	RenderCommand::Flush();
}

// Wait for pending GPU work to complete.
void Renderer::WaitForGpu()
{
	// Schedule a Signal command in the queue.
	HR(DXCore::GetCommandQueue()->Signal(DXCore::GetFence().Get(), m_FrameFenceValues[m_FrameIndex]));

	// Wait until the fence has been processed.
	HR(DXCore::GetFence()->SetEventOnCompletion(m_FrameFenceValues[m_FrameIndex], DXCore::GetFenceEvent()));
	WaitForSingleObjectEx(DXCore::GetFenceEvent(), INFINITE, FALSE);

	// Increment the fence value for the current frame.
	m_FrameFenceValues[m_FrameIndex]++;
}

void Renderer::WaitAndSync()
{
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = m_FrameFenceValues[m_FrameIndex];
	HR(DXCore::GetCommandQueue()->Signal(DXCore::GetFence().Get(), currentFenceValue));

	// Update the frame index.
	m_FrameIndex = Window::Get().GetCurrentBackbufferIndex() % NR_OF_FRAMES;
	
	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (DXCore::GetFence()->GetCompletedValue() < m_FrameFenceValues[m_FrameIndex])
	{
		PIXBeginEvent(400, "Renderer::Sync");
		HR(DXCore::GetFence()->SetEventOnCompletion(m_FrameFenceValues[m_FrameIndex], DXCore::GetFenceEvent()));
		WaitForSingleObjectEx(DXCore::GetFenceEvent(), INFINITE, FALSE);
		PIXEndEvent();
	}

	// Set the fence value for the next frame.
	m_FrameFenceValues[m_FrameIndex] = currentFenceValue + 1;
}

void Renderer::CreateDepthBuffer() noexcept
{
	auto [width, height] {Window::Get().GetDimensions()};

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 0u;
	heapProperties.VisibleNodeMask = 0u;

	D3D12_RESOURCE_DESC DepthBufferResourceDesc{};
	DepthBufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	DepthBufferResourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	DepthBufferResourceDesc.Width = width;
	DepthBufferResourceDesc.Height = height;
	DepthBufferResourceDesc.DepthOrArraySize = 1u;
	DepthBufferResourceDesc.MipLevels = 1u;
	DepthBufferResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	DepthBufferResourceDesc.SampleDesc = { 1u, 0u };
	DepthBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	DepthBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;

	HR(DXCore::GetDevice()->CreateCommittedResource
	(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&DepthBufferResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(&m_pDepthBuffer)
	));
	HR(m_pDepthBuffer->SetName(L"Main Depth Buffer"));

	m_pDSVDescriptorHeap = std::make_unique<DescriptorHeap>(1u, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, false);

	STDCALL(DXCore::GetDevice()->CreateDepthStencilView
	(
		m_pDepthBuffer.Get(), 
		nullptr, 
		m_pDSVDescriptorHeap->GetCPUStartHandle()
	));
}

void Renderer::CreateRootSignature() noexcept
{
	std::vector<D3D12_ROOT_PARAMETER> rootParameters;

	D3D12_DESCRIPTOR_RANGE descriptorRange = {};
	descriptorRange.BaseShaderRegister = 1u;
	descriptorRange.RegisterSpace = 0u;
	descriptorRange.NumDescriptors = 1u;
	descriptorRange.OffsetInDescriptorsFromTableStart = 0u;
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;

	D3D12_ROOT_PARAMETER transformsRootParameterVS = {};
	transformsRootParameterVS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	transformsRootParameterVS.DescriptorTable.NumDescriptorRanges = 1u;
	transformsRootParameterVS.DescriptorTable.pDescriptorRanges = &descriptorRange;
	transformsRootParameterVS.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters.push_back(transformsRootParameterVS);

	D3D12_ROOT_PARAMETER vertexBufferSRVParameter = {};
	vertexBufferSRVParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;		
	vertexBufferSRVParameter.Descriptor.ShaderRegister = 0u;					
	vertexBufferSRVParameter.Descriptor.RegisterSpace = 0u;						
	vertexBufferSRVParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters.push_back(vertexBufferSRVParameter);

	D3D12_ROOT_PARAMETER indexBufferSRVParameter = {};
	indexBufferSRVParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;		//The type is a SRV.
	indexBufferSRVParameter.Descriptor.ShaderRegister = 1u;						//Basically :register(1,0)
	indexBufferSRVParameter.Descriptor.RegisterSpace = 0u;						//^
	indexBufferSRVParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;	//This should be visible in the VERTEX SHADER.
	rootParameters.push_back(indexBufferSRVParameter);

	D3D12_ROOT_PARAMETER vpRootParameterVS = {};
	vpRootParameterVS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	vpRootParameterVS.Constants.Num32BitValues = 4*4;
	vpRootParameterVS.Constants.ShaderRegister = 0u;
	vpRootParameterVS.Constants.RegisterSpace = 0u;
	vpRootParameterVS.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters.push_back(vpRootParameterVS);

	//For raytracing.
	D3D12_ROOT_PARAMETER accelerationStructureSRVParameter = {};
	accelerationStructureSRVParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	accelerationStructureSRVParameter.Descriptor.ShaderRegister = 0u;
	accelerationStructureSRVParameter.Descriptor.RegisterSpace = 1u;
	accelerationStructureSRVParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters.push_back(accelerationStructureSRVParameter);

	D3D12_ROOT_PARAMETER vpInversePS = {};
	vpInversePS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	vpInversePS.Constants.Num32BitValues = 4 * 4; //The matrix + the 2 projection matrix elements.
	vpInversePS.Constants.ShaderRegister = 0u;
	vpInversePS.Constants.RegisterSpace = 1u;
	vpInversePS.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters.push_back(vpInversePS);

	D3D12_ROOT_PARAMETER objectColorPS = {};
	objectColorPS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	objectColorPS.Constants.Num32BitValues = 4;
	objectColorPS.Constants.ShaderRegister = 1u;
	objectColorPS.Constants.RegisterSpace = 1u;
	objectColorPS.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters.push_back(objectColorPS);
	
	D3D12_ROOT_PARAMETER cameraPS = {};
	cameraPS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	cameraPS.Constants.Num32BitValues = 3 + 1;
	cameraPS.Constants.ShaderRegister = 2u;
	cameraPS.Constants.RegisterSpace = 1u;
	cameraPS.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters.push_back(cameraPS);
	
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDescriptor = {};
	rootSignatureDescriptor.NumParameters = static_cast<UINT>(rootParameters.size());
	rootSignatureDescriptor.pParameters = rootParameters.data();
	rootSignatureDescriptor.NumStaticSamplers = 0u;
	rootSignatureDescriptor.pStaticSamplers = nullptr;
	rootSignatureDescriptor.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	Microsoft::WRL::ComPtr<ID3DBlob> pRootSignatureBlob = nullptr;
	SERIALIZE_ROOT_SIGNATURE(rootSignatureDescriptor, pRootSignatureBlob);
	HR(DXCore::GetDevice()->CreateRootSignature
	(
		0u,
		pRootSignatureBlob->GetBufferPointer(),
		pRootSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&m_pRootSignature)
	));
}

void Renderer::CreatePipelineStateObject() noexcept
{
	//We need a rasterizer descriptor:
	D3D12_RASTERIZER_DESC rasterizerDescriptor = {};
	rasterizerDescriptor.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDescriptor.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDescriptor.FrontCounterClockwise = FALSE;
	rasterizerDescriptor.DepthBias = 0;
	rasterizerDescriptor.DepthBiasClamp = 0.0f;
	rasterizerDescriptor.SlopeScaledDepthBias = 0.0f;
	rasterizerDescriptor.DepthClipEnable = TRUE;
	rasterizerDescriptor.MultisampleEnable = FALSE;
	rasterizerDescriptor.AntialiasedLineEnable = FALSE;
	rasterizerDescriptor.ForcedSampleCount = 0u;
	rasterizerDescriptor.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	//We also need a blend descriptor:
	D3D12_RENDER_TARGET_BLEND_DESC blendDescriptor = {};
	blendDescriptor.BlendEnable = FALSE;
	blendDescriptor.LogicOpEnable = FALSE;
	blendDescriptor.SrcBlend = D3D12_BLEND_ONE;
	blendDescriptor.DestBlend = D3D12_BLEND_ZERO;
	blendDescriptor.BlendOp = D3D12_BLEND_OP_ADD;
	blendDescriptor.SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDescriptor.DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDescriptor.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDescriptor.LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDescriptor.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//And a depth stencil descriptor:
	D3D12_DEPTH_STENCIL_DESC depthStencilDescriptor = {};
	depthStencilDescriptor.DepthEnable = TRUE;
	depthStencilDescriptor.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDescriptor.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDescriptor.StencilEnable = FALSE;
	depthStencilDescriptor.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	depthStencilDescriptor.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	depthStencilDescriptor.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDescriptor.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDescriptor.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDescriptor.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDescriptor.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDescriptor.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDescriptor.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStencilDescriptor.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	//We also need a Stream Output Descriptor:
	D3D12_STREAM_OUTPUT_DESC streamOutputDescriptor = {};
	streamOutputDescriptor.pSODeclaration = nullptr;
	streamOutputDescriptor.NumEntries = 0u;
	streamOutputDescriptor.pBufferStrides = nullptr;
	streamOutputDescriptor.NumStrides = 0u;
	streamOutputDescriptor.RasterizedStream = 0u;

	//We need the shaders:
	//First we compile them using the dxc compiler.
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = CompileShader(L"VertexShader.hlsl", L"main", L"vs_6_5");
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = CompileShader(L"PixelShader.hlsl", L"main", L"ps_6_5");

	//We now create the Graphics Pipe line state, the PSO:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescriptor = { 0 };
	std::vector<DXGI_FORMAT> rtvFormats = { DXGI_FORMAT_R8G8B8A8_UNORM };
	psoDescriptor.pRootSignature = m_pRootSignature.Get();
	psoDescriptor.VS.pShaderBytecode = vertexShaderBlob->GetBufferPointer();
	psoDescriptor.VS.BytecodeLength = vertexShaderBlob->GetBufferSize();
	psoDescriptor.PS.pShaderBytecode = pixelShaderBlob->GetBufferPointer();
	psoDescriptor.PS.BytecodeLength = pixelShaderBlob->GetBufferSize();

	psoDescriptor.SampleMask = UINT_MAX;
	psoDescriptor.RasterizerState = rasterizerDescriptor;
	psoDescriptor.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDescriptor.NumRenderTargets = static_cast<UINT>(rtvFormats.size());

	psoDescriptor.BlendState.AlphaToCoverageEnable = false;
	psoDescriptor.BlendState.IndependentBlendEnable = false;
	psoDescriptor.RTVFormats[0] = rtvFormats[0];
	psoDescriptor.BlendState.RenderTarget[0] = blendDescriptor;

	psoDescriptor.SampleDesc.Count = 1u;
	psoDescriptor.SampleDesc.Quality = 0u;
	psoDescriptor.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDescriptor.DepthStencilState = depthStencilDescriptor;
	psoDescriptor.StreamOutput = streamOutputDescriptor;
	psoDescriptor.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HR(DXCore::GetDevice()->CreateGraphicsPipelineState(&psoDescriptor, IID_PPV_ARGS(&m_pPSO)));
}

Microsoft::WRL::ComPtr<IDxcBlob> Renderer::CompileShader(const std::wstring& filepath, const std::wstring& entryPoint, const std::wstring& target) noexcept
{
	//Find the file name.
	uint32_t dotPos = static_cast<uint32_t>(filepath.find(L".", 0));
	std::wstring name = filepath.substr(0, dotPos);

	//Compiler and utils should be kept in some sort of shader handler ideally.
	Microsoft::WRL::ComPtr<IDxcCompiler3> pCompiler = nullptr;
	HR(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(pCompiler.GetAddressOf())));
	Microsoft::WRL::ComPtr<IDxcUtils> pUtils = nullptr;
	HR(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.GetAddressOf())));

	Microsoft::WRL::ComPtr<IDxcBlobEncoding> pSource = nullptr;
	uint32_t codePage = CP_UTF8;
	HR(pUtils->LoadFile(filepath.c_str(), &codePage, pSource.GetAddressOf()));
	
	//Shows all arguments
	std::vector<LPCWSTR> arguments = {};

	//E for entrypoint.
	arguments.push_back(L"-E");
	arguments.push_back(entryPoint.c_str());

	//T for target profile (shader version)
	arguments.push_back(L"-T");
	arguments.push_back(target.c_str());

	arguments.push_back(DXC_ARG_PACK_MATRIX_COLUMN_MAJOR);

#if defined(_DEBUG)
	arguments.push_back(L"-Od"); //Disable optimizations.
	//arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);
	arguments.push_back(DXC_ARG_DEBUG);

	//Write debug information to the given file
	arguments.push_back(L"-Fd");
	std::wstring nameDebug = name + L"_debug";
	arguments.push_back(nameDebug.c_str());

	//Write errors and warnings to the given file
	arguments.push_back(L"-Fe");
	std::wstring nameError = name + L"_error";
	arguments.push_back(nameError.c_str());
#else
	arguments.push_back(L"-O3"); //Optimization level.
#endif

	//To be able to strip reflection data and pdb's. They are separated from the shader object, therefore reducing the shader object's size.
	arguments.push_back(L"-Qstrip_debug");
	arguments.push_back(L"-Qstrip_reflect");

	//Macros can be defined in strings and sent to the shader if we want.
	/*
	for (const std::wstring& define : defines)
	{
		arguments.push_back(L"-D");
		arguments.push_back(define.c_str());
	}
	*/

	DxcBuffer sourceBuffer = {};
	sourceBuffer.Ptr = pSource->GetBufferPointer();
	sourceBuffer.Size = pSource->GetBufferSize();
	sourceBuffer.Encoding = 0;

	
	Microsoft::WRL::ComPtr<IDxcResult> pCompileResult = nullptr;
	HR(pCompiler->Compile(&sourceBuffer, arguments.data(), (uint32_t)arguments.size(), nullptr, IID_PPV_ARGS(pCompileResult.GetAddressOf())));
	
#if defined(_DEBUG)
	//Handle errors.
	Microsoft::WRL::ComPtr<IDxcBlobUtf8> pErrors = nullptr;
	HR(pCompileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(pErrors.GetAddressOf()), nullptr));
	//DBG_ASSERT(pErrors && pErrors->GetStringLength() > 0, (char*)pErrors->GetBufferPointer()); //? Dont know how to print this.

	//Get the Debug data.
	Microsoft::WRL::ComPtr<IDxcBlob> pDebugData = nullptr;
	//This contains the path that is baked into the shader object to refer to the part in question.
	//So if you want to save the PDBs to a separate file, use this name so that Pix will know where to find it.
	Microsoft::WRL::ComPtr<IDxcBlobUtf16> pDebugDataPath = nullptr;
	HR(pCompileResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(pDebugData.GetAddressOf()), pDebugDataPath.GetAddressOf()));
	//How do we print the debug data?
	
	//Get the reflection data.
	Microsoft::WRL::ComPtr<IDxcBlob> pReflectionData = nullptr;
	HR(pCompileResult->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(pReflectionData.GetAddressOf()), nullptr));

	DxcBuffer reflectionBuffer = {};
	reflectionBuffer.Ptr = pReflectionData->GetBufferPointer();
	reflectionBuffer.Size = pReflectionData->GetBufferSize();
	reflectionBuffer.Encoding = 0;

	Microsoft::WRL::ComPtr<ID3D12ShaderReflection> pShaderReflection = nullptr;
	HR(pUtils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(pShaderReflection.GetAddressOf())));
	//How do we use the reflectiondata?
#endif

	Microsoft::WRL::ComPtr<IDxcBlob> pResultBlob = nullptr;
	pCompileResult->GetResult(pResultBlob.GetAddressOf());

	return pResultBlob;
}

//The CSO path get sent in here and then returns the blob.
Microsoft::WRL::ComPtr<ID3DBlob> Renderer::LoadCSO(const std::wstring& filepath) noexcept
{
	std::ifstream file(filepath, std::ios::binary);
	DBG_ASSERT(file.is_open(), "Error! Could not open CSO file!");

	file.seekg(0, std::ios_base::end);
	size_t size = static_cast<size_t>(file.tellg());
	file.seekg(0, std::ios_base::beg);

	Microsoft::WRL::ComPtr<ID3DBlob> toReturn = nullptr;
	HR(D3DCreateBlob(size, &toReturn));

	file.read(static_cast<char*>(toReturn->GetBufferPointer()), size);
	file.close();

	return toReturn;
}

void Renderer::CreateViewportAndScissorRect() noexcept
{
	auto [width, height] = Window::Get().GetDimensions();

	m_ViewPort.TopLeftX = 0u;
	m_ViewPort.TopLeftY = 0u;
	m_ViewPort.MinDepth = 0.0f;
	m_ViewPort.MaxDepth = 1.0f; //?
	m_ViewPort.Width = static_cast<float>(width);
	m_ViewPort.Height = static_cast<float>(height);

	m_ScissorRect.left = 0u;
	m_ScissorRect.top = 0u;
	m_ScissorRect.right = static_cast<LONG>(m_ViewPort.Width);
	m_ScissorRect.bottom = static_cast<LONG>(m_ViewPort.Height);
}
