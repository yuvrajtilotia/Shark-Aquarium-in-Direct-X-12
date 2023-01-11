#include "pch.h"
#include "Scene.h"
#include "Window.h"

void Scene::Initialize() noexcept
{
	m_pRayTracingManager = std::make_unique<RayTracingManager>();
	//Use this to randomize colors.
	using t_clock = std::chrono::high_resolution_clock;
	std::default_random_engine generator(static_cast<UINT>(t_clock::now().time_since_epoch().count()));
	std::uniform_real_distribution<float> distributionColor(0.0f, 1.0f);
	std::uniform_real_distribution<float> distributionSize(0.5f, 2.0f);
	std::uniform_int_distribution<int> distributionBehaviour(0, 3);
	std::uniform_real_distribution<float> distributionPos(-10.0f, 150.0f);
	//Create objects here.-40.0f, 60.0f, 80.0f
	AddVertexObject("Tri", DirectX::XMVectorSet(-10.0f, 0.0f, 50.0f, 1.0f), DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), 10.0f, RESIZE, DirectX::XMFLOAT4(distributionColor(generator), distributionColor(generator), distributionColor(generator), 1.0f));
	AddVertexObject("Rec", DirectX::XMVectorSet(10.0f, 0.0f, 90.0f, 1.0f), DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), 20.0f, MOVEBACKANDFORTH, DirectX::XMFLOAT4(distributionColor(generator), distributionColor(generator), distributionColor(generator), 1.0f));
	AddVertexObject("Rec", DirectX::XMVectorSet(10.0f, 0.0f, 100.0f, 1.0f), DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), 20.0f, SPIN, DirectX::XMFLOAT4(distributionColor(generator), distributionColor(generator), distributionColor(generator), 1.0f));

	//AddVertexObject("Models/Spaceship.obj", DirectX::XMVectorSet(50.0f, 50.0f, 80.0f, 1.0f), DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), 0.5f, NONE, DirectX::XMFLOAT4(distributionColor(generator), distributionColor(generator), distributionColor(generator), 1.0f));
	//AddVertexObject("Models/Spaceship.obj", DirectX::XMVectorSet(-30.0f, 80.0f, 40.0f, 1.0f), DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), 0.5f, NONE, DirectX::XMFLOAT4(distributionColor(generator), distributionColor(generator), distributionColor(generator), 1.0f));
	AddVertexObject("Models/Shark.obj", DirectX::XMVectorSet(50.0f, 0.0f, 80.0f, 1.0f), DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), 2.0f, SPIN, DirectX::XMFLOAT4(distributionColor(generator), distributionColor(generator), distributionColor(generator), 1.0f));
	AddVertexObject("Models/Shark.obj", DirectX::XMVectorSet(-20.0f, 0.0f, 50.0f, 1.0f), DirectX::XMVectorSet(0.0f, (float)M_PI / 2, 0.0f, 0.0f), 2.0f, RESIZE, DirectX::XMFLOAT4(distributionColor(generator), distributionColor(generator), distributionColor(generator), 1.0f));
	for (uint32_t i{ 0u }; i < 100; i++)
	{
		int behaviour = distributionBehaviour(generator);
		switch (behaviour)
		{
		case 0:
		{
			AddVertexObject("Models/Shark.obj", DirectX::XMVectorSet(distributionPos(generator), distributionPos(generator), distributionPos(generator), 1.0f), DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), distributionSize(generator), NONE, DirectX::XMFLOAT4(distributionColor(generator), distributionColor(generator), distributionColor(generator), distributionColor(generator)));
			break;
		}
		case 1:
		{
			AddVertexObject("Models/Shark.obj", DirectX::XMVectorSet(distributionPos(generator), distributionPos(generator), distributionPos(generator), 1.0f), DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), distributionSize(generator), SPIN, DirectX::XMFLOAT4(distributionColor(generator), distributionColor(generator), distributionColor(generator), distributionColor(generator)));
			break;
		}
		case 2:
		{
			AddVertexObject("Models/Shark.obj", DirectX::XMVectorSet(distributionPos(generator), distributionPos(generator), distributionPos(generator), 1.0f), DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), distributionSize(generator), MOVEBACKANDFORTH, DirectX::XMFLOAT4(distributionColor(generator), distributionColor(generator), distributionColor(generator), distributionColor(generator)));
			break;
		}
		case 3:
		{
			AddVertexObject("Models/Shark.obj", DirectX::XMVectorSet(distributionPos(generator), distributionPos(generator), distributionPos(generator), 1.0f), DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), distributionSize(generator), RESIZE, DirectX::XMFLOAT4(distributionColor(generator), distributionColor(generator), distributionColor(generator), distributionColor(generator)));
			break;
		}
		default:
		{
			break;
		}
		}
	}
	//Create the "room"
	//Floor
	AddVertexObject("Rec", DirectX::XMVectorSet(0.0f, -10.0f, 50.0f, 1.0f), DirectX::XMVectorSet((float)M_PI / 2.0f, 0.0f, 0.0f, 0.0f), 200.0f, NONE, DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	////Wall behind
	AddVertexObject("Rec", DirectX::XMVectorSet(0.0f, 90.0f, -50.0f, 1.0f), DirectX::XMVectorSet(0.0f, (float)M_PI, 0.0f, 0.0f), 200.0f, NONE, DirectX::XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f));
	////Wall in front
	AddVertexObject("Rec", DirectX::XMVectorSet(0.0f, 90.0f, 150.0f, 1.0f), DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), 200.0f, NONE, DirectX::XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f));
	////Wall to the left
	AddVertexObject("Rec", DirectX::XMVectorSet(-100.0f, 90.0f, 50.0f, 1.0f), DirectX::XMVectorSet(0.0f, (float)-M_PI / 2.0f, 0.0f, 0.0f), 200.0f, NONE, DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f));
	////Wall to the right
	AddVertexObject("Rec", DirectX::XMVectorSet(100.0f, 90.0f, 50.0f, 1.0f), DirectX::XMVectorSet(0.0f, (float)M_PI / 2.0f, 0.0f, 0.0f), 200.0f, NONE, DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f));

	auto pCommandAllocator = DXCore::GetCommandAllocators()[0];
	auto pCommandList = DXCore::GetCommandList();
	auto pDevice = DXCore::GetDevice();

	HR(pCommandList->Close());
	ID3D12CommandList* commandLists[] = { pCommandList.Get() };
	STDCALL(DXCore::GetCommandQueue()->ExecuteCommandLists(ARRAYSIZE(commandLists), commandLists));
	RenderCommand::Flush();
	
	HR(pCommandAllocator->Reset());
	HR(pCommandList->Reset(pCommandAllocator.Get(), nullptr));

	m_pRayTracingManager->Initialize(m_UniqueModels, m_Objects, m_TotalMeshes);

	HR(pCommandList->Close());
	STDCALL(DXCore::GetCommandQueue()->ExecuteCommandLists(ARRAYSIZE(commandLists), commandLists));
	RenderCommand::Flush();

	HR(pCommandAllocator->Reset());
	HR(pCommandList->Reset(pCommandAllocator.Get(), nullptr));
}

void Scene::Update(bool rayTraceBool, float deltaTime) noexcept
{
	UINT fif = Window::Get().GetCurrentFrameInFlightIndex();
	auto pCommandAllocator = DXCore::GetCommandAllocators()[fif];
	auto pCommandList = DXCore::GetCommandList();

	HR(pCommandAllocator->Reset());
	HR(pCommandList->Reset(pCommandAllocator.Get(), nullptr));
	
	//Update all objects.
	for (auto& modelInstances : m_Objects)
	{
		for (auto& object : modelInstances.second)
		{
			const std::vector<std::unique_ptr<Mesh>>& objectMeshes = object->GetModel()->GetMeshes();
			for (uint32_t i{ 0u }; i < objectMeshes.size(); i++)
			{
				object->Update(deltaTime);
			}
		}
	}
	if (rayTraceBool)
	{
		//Update the top level acceleration structure.
		m_pRayTracingManager->UpdateInstances(m_UniqueModels, m_Objects, m_TotalMeshes);
	}
}

void Scene::AddVertexObject(const std::string path, DirectX::XMVECTOR pos, DirectX::XMVECTOR rot, float scale, UpdateType updateType, DirectX::XMFLOAT4 color)
{	
	std::shared_ptr<Model> tempModel = nullptr;
	bool newModel = false;

	//Check if the requested model is new or not.
	if (m_UniqueModels.find(path) == m_UniqueModels.end())
	{
		//If it is a new model we have to create it and initialize (load it using Assimp) the model.
		//It is then inserted into our unique models map.
		tempModel = std::make_shared<Model>();
		tempModel->Initialize(path);
		m_UniqueModels.insert(std::pair(path, tempModel));

		newModel = true;
	}
	else
	{
		//Otherwise we simply fetch the shared_ptr to the unique model.
		tempModel = m_UniqueModels[path];
	}
	m_TotalMeshes += (uint32_t)(tempModel->GetMeshes().size());
	for (uint32_t i{ 0u }; i < tempModel->GetMeshes().size(); ++i)
	{
		m_TotalNrOfVertices += tempModel->GetMeshes()[i]->GetVertexCount();
		m_TotalNrOfIndices += tempModel->GetMeshes()[i]->GetIndexCount();
	}

	//No matter the case we want to create a new object using the fetched model.
	std::shared_ptr<VertexObject> tempObject = std::make_shared<VertexObject>();
	tempObject->Initialize(
		std::move(tempModel),
		pos,
		rot,
		scale,
		updateType,
		color
	);
	
	if (newModel)
	{
		//If the model was new we have to create a new vector that we add the object to. We then insert the vector into our objects map.
		std::vector<std::shared_ptr<VertexObject>> objects = {};
		objects.push_back(std::move(tempObject));
		m_Objects.insert(std::pair(path, objects));
	}
	else
	{
		//If the model already existed we simply push back the new object to the already existing vector.
		m_Objects[path].push_back(std::move(tempObject));
	}
	
	//Increment the total number of objects.
	m_TotalObjects++;
}