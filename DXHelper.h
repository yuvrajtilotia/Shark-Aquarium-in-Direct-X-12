#pragma once
class DXHelper;
#ifdef CreateWindow
	#undef CreateWindow
#endif

#if defined(_DEBUG) 
	#define APP_NAME L"D3D12 Project - Debug"
	#define INIT_MEMORY_LEAK_DETECTION _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )

	#define DBG_ASSERT(expression, message)	\
	{	\
		if (!(expression)) \
		{	\
			std::cout << "Assertion failed on expression " << #expression << "\n";	\
			std::cout << "Error message: " << message << "\n";	\
			std::cout << "File: " << __FILE__ << "\n";	\
			std::cout << "Function: " << __FUNCTION__ << "\n";	\
			std::cout << "Line:" << __LINE__ << "\n";	\
			__debugbreak();	\
		}	\
	}
#else
	#define APP_NAME L"D3D12 Project - Release"
	#define INIT_MEMORY_LEAK_DETECTION  
	#define DBG_NEW new
	#define DBG_ASSERT(expression, message)
#endif

#pragma region "DX Debug Calls"
//Error handling for hr-values.
#if defined(_DEBUG)
#ifndef HR
	#define HR(function)	\
	{	\
		if (DXHelper::GetInfoQueue())	\
		{	\
			DXHelper::GetInfoQueue()->ClearStoredMessages();	\
		}	\
		HRESULT hr = (function);	\
		if (FAILED(hr)){	\
			std::cout << "DX12 has encountered a fatal error.\n";	\
			std::cout << "File: " << __FILE__ << "\n";	\
			std::cout << "Function: " << __FUNCTION__ << "\n";	\
			std::cout << "Line:" << __LINE__ << "\n";	\
			_com_error comError(hr);	\
			std::wcout << comError.ErrorMessage();	\
			if (DXHelper::GetInfoQueue())	\
			{	\
				std::cout << "DX12 error message: ";	\
				for (uint32_t messageIndex{0u}; messageIndex < DXHelper::GetInfoQueue()->GetNumStoredMessages(); messageIndex++)	\
				{	\
					SIZE_T messageLength = 0;	\
					DBG_ASSERT((DXHelper::GetInfoQueue()->GetMessage(0, NULL, &messageLength) == S_OK), "Failed to get info queue message");	\
					std::unique_ptr<D3D12_MESSAGE> pMessage = std::unique_ptr<D3D12_MESSAGE>(DBG_NEW D3D12_MESSAGE[messageLength]);	\
					DBG_ASSERT((DXHelper::GetInfoQueue()->GetMessage(0, pMessage.get(), &messageLength) == S_OK), "Failed to get info queue message");	\
					std::cout << pMessage->pDescription << "\n";	\
				}	\
			}	\
			__debugbreak();	\
		}	\
	}
#endif
#else
	#ifndef HR
		#define HR(function) function
	#endif
#endif

//stdcall error handling.
#if defined(_DEBUG) || defined(DEBUG)
#ifndef STDCALL
	#define STDCALL(function)	\
	{	\
		DXHelper::GetInfoQueue()->ClearStoredMessages();	\
		(function);	\
		if (DXHelper::GetInfoQueue()->GetNumStoredMessages() > 0)	\
		{	\
			std::cout << "DX12 has encountered a fatal error.\n";	\
			std::cout << "File: " << __FILE__ << "\n";	\
			std::cout << "Function: " << __FUNCTION__ << "\n";	\
			std::cout << "Line:" << __LINE__ << "\n";	\
			std::cout << "DX12 error message: ";	\
			for (uint32_t messageIndex{ 0u }; messageIndex < DXHelper::GetInfoQueue()->GetNumStoredMessages(); messageIndex++)	\
			{	\
				SIZE_T messageLength = 0;	\
				DBG_ASSERT((DXHelper::GetInfoQueue()->GetMessage(0, NULL, &messageLength) == S_OK), "Failed to get info queue message.");	\
				std::unique_ptr<D3D12_MESSAGE> pMessage = std::unique_ptr<D3D12_MESSAGE>(DBG_NEW D3D12_MESSAGE[messageLength]);	\
				DBG_ASSERT((DXHelper::GetInfoQueue()->GetMessage(0, pMessage.get(), &messageLength) == S_OK), "Failed to get info queue message.");	\
				std::cout << pMessage->pDescription << "\n";	\
			}	\
			__debugbreak();	\
		}	\
	}
#endif
#else
	#ifndef STDCALL
		#define STDCALL(function) function
	#endif
#endif

#if defined(_DEBUG) || defined(DEBUG)
#ifndef SERIALIZE_ROOT_SIGNATURE
#define SERIALIZE_ROOT_SIGNATURE(rootSignatureDescriptor, pRootSignatureBlob)	\
		{	\
			Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob = nullptr;	\
			DXHelper::GetInfoQueue()->ClearStoredMessages();	\
			HRESULT hr3 = D3D12SerializeRootSignature(&rootSignatureDescriptor, D3D_ROOT_SIGNATURE_VERSION_1_0, &pRootSignatureBlob, &pErrorBlob);	\
			if (FAILED(hr3))	\
			{	\
				std::cout << "ERROR: DX12 has encountered a critical error:" << "\n";	\
				std::cout << "File: " << __FILE__ << "\n";	\
				std::cout << "Function: " << __FUNCTION__ << "\n";	\
				std::cout << "Line: " << __LINE__ << "\n";	\
				_com_error comError2(hr3);	\
				std::wcout << comError2.ErrorMessage();	\
				if (pErrorBlob)	\
					std::cout << "Error blob message: " << pErrorBlob->GetBufferPointer() << "\n";	\
				if (DXHelper::GetInfoQueue()->GetNumStoredMessages() > 0)	\
					std::cout << "D3D12 error message: ";	\
				for (uint32_t i{0u}; i < DXHelper::GetInfoQueue()->GetNumStoredMessages(); i++)	\
				{	\
					size_t messageLength2{0u};	\
					HR(DXHelper::GetInfoQueue()->GetMessage(i, nullptr, &messageLength2));	\
					std::unique_ptr<D3D12_MESSAGE> pMessage2 = std::unique_ptr<D3D12_MESSAGE>(DBG_NEW D3D12_MESSAGE[messageLength2]);	\
					HR(DXHelper::GetInfoQueue()->GetMessage(i, pMessage2.get(), &messageLength2));	\
					std::cout <<  pMessage2->pDescription << "\n";	\
				}	\
				__debugbreak();	\
			}	\
		}
#endif
#else
#ifndef SERIALIZE_ROOT_SIGNATURE
#define SERIALIZE_ROOT_SIGNATURE(rootSignatureDescriptor, pRootSignatureBlob)	\
			{	\
				D3D12SerializeRootSignature(&rootSignatureDescriptor, D3D_ROOT_SIGNATURE_VERSION_1_0, &pRootSignatureBlob, nullptr);	\
			}
#endif
#endif

#if defined(_DEBUG) || defined(DEBUG)
#ifndef COMPILE_FROM_FILE
#define COMPILE_FROM_FILE(fileName, entryPoint, shaderVersion, shaderBlob)	\
		{	\
			Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob2 = nullptr;	\
			DXHelper::GetInfoQueue()->ClearStoredMessages();	\
			HRESULT hr4 = D3DCompileFromFile(fileName, \
											 nullptr, \
											 D3D_COMPILE_STANDARD_FILE_INCLUDE, \
											 entryPoint, \
											 shaderVersion,	\
											 D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, \
											 0u, \
											 &shaderBlob, \
											 &pErrorBlob2);	\
			if (FAILED(hr4))	\
			{	\
				std::cout << "ERROR: DX12 has encountered a critical error:" << "\n";	\
				std::cout << "File: " << __FILE__ << "\n";	\
				std::cout << "Function: " << __FUNCTION__ << "\n";	\
				std::cout << "Line: " << __LINE__ << "\n";	\
				_com_error comError3(hr4);	\
				std::wcout << comError3.ErrorMessage();	\
				if (pErrorBlob2)	\
					std::cout << "Error blob message: " << pErrorBlob2->GetBufferPointer() << "\n";	\
				if (DXHelper::GetInfoQueue()->GetNumStoredMessages() > 0)	\
					std::cout << "D3D12 error message: ";	\
				for (uint32_t i{ 0u }; i < DXHelper::GetInfoQueue()->GetNumStoredMessages(); i++)	\
				{	\
					size_t messageLength3{ 0u };	\
					HR(DXHelper::GetInfoQueue()->GetMessage(i, nullptr, &messageLength3));	\
					std::unique_ptr<D3D12_MESSAGE> pMessage3 = std::unique_ptr<D3D12_MESSAGE>(DBG_NEW D3D12_MESSAGE[messageLength3]);	\
					HR(DXHelper::GetInfoQueue()->GetMessage(i, pMessage3.get(), &messageLength3));	\
					std::cout << pMessage3->pDescription << "\n";	\
				}	\
					__debugbreak();	\
			}	\
		}
#endif
#else
#ifndef COMPILE_FROM_FILE
#define COMPILE_FROM_FILE(fileName, entryPoint, shaderVersion, shaderBlob)	\
		{	\
			D3DCompileFromFile(fileName, \
				nullptr, \
				D3D_COMPILE_STANDARD_FILE_INCLUDE, \
				entryPoint, \
				shaderVersion,	\
				0u, \
				0u, \
				& shaderBlob, \
				nullptr);	\
		}
#endif
#endif

#pragma endregion

#if defined(_DEBUG)
class DXHelper
{
public:
	DXHelper() noexcept = default;
	~DXHelper() noexcept = default;
	[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<ID3D12InfoQueue>& GetInfoQueue() noexcept { return s_pInfoQueue; }
	static void InitializeInfoQueue(const Microsoft::WRL::ComPtr<ID3D12Device5> pDevice) noexcept
	{
		DBG_ASSERT(pDevice, "Device is not initalized.");
		DBG_ASSERT(!s_pInfoQueue, "Info queue is already initialized.");
		pDevice->QueryInterface(IID_PPV_ARGS(&s_pInfoQueue));
	}
private:
	static Microsoft::WRL::ComPtr<ID3D12InfoQueue> s_pInfoQueue;
};
#endif

#define NR_OF_FRAMES 3
#define NR_OF_BACKBUFFERS 3

struct ConstantBufferView
{
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, NR_OF_FRAMES> pResources = {};
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, NR_OF_FRAMES> SrcHandles = {};
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, NR_OF_FRAMES> DstHandles = {};
	std::array<D3D12_GPU_DESCRIPTOR_HANDLE, NR_OF_FRAMES> GpuHandles = {};
};