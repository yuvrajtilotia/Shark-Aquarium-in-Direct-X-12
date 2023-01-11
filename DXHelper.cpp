#include "pch.h"
#include "DXHelper.h"

#if defined(_DEBUG)
Microsoft::WRL::ComPtr<ID3D12InfoQueue> DXHelper::s_pInfoQueue{ nullptr };
#endif