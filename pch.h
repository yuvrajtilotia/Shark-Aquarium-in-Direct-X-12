#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <wrl/client.h>
#include <comdef.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <dxc/dxcapi.h>

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <stdint.h>
#include <crtdbg.h>
#include <assert.h>
#include <bitset>
#include <chrono>
#include <array>
#include <codecvt>
#include <locale>
#include <fstream>
#include <random>
#define _USE_MATH_DEFINES
#include <math.h>
#include <iomanip>

#include "DXHelper.h"

#include "imgui.h"
