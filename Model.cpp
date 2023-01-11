#include "pch.h"
#include "Model.h"

void Model::Initialize(const std::string path) noexcept
{
	m_Name = path;
	if (path == "Tri")
	{
		LoadTri();
	}
	else if (path == "Rec")
	{
		LoadRec();
	}
	else
	{
		LoadModel();
	}
}

void Model::LoadTri() noexcept
{
	std::vector<Vertex> vertices = {};
	std::vector<uint32_t> indices = {};

	//Set up the triangle vertices and indices.
	Vertex v1 = {};
	v1.pos = DirectX::XMFLOAT3{ -0.5f, -0.5f, 0.0f };
	v1.normal = DirectX::XMFLOAT3{ 0.0f, 0.0f, -1.0f };

	Vertex v2 = {};
	v2.pos = DirectX::XMFLOAT3{ 0.0f, 0.5f, 0.0f };
	v2.normal = DirectX::XMFLOAT3{ 0.0f, 0.0f, -1.0f };

	Vertex v3 = {};
	v3.pos = DirectX::XMFLOAT3{ 0.5f, -0.5f, 0.0f };
	v3.normal = DirectX::XMFLOAT3{ 0.0f, 0.0f, -1.0f };

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);

	indices.push_back(0u);
	indices.push_back(1u);
	indices.push_back(2u);

	m_Meshes.push_back(std::make_unique<Mesh>(vertices, indices));
}

void Model::LoadRec() noexcept
{
	std::vector<Vertex> vertices = {};
	std::vector<uint32_t> indices = {};

	//Set up the rec vertices and indices.
	Vertex v1 = {};
	v1.pos = DirectX::XMFLOAT3{ -0.5f, -0.5f, 0.0f };
	//Random color atm.
	using t_clock = std::chrono::high_resolution_clock;
	std::default_random_engine generator(static_cast<UINT>(t_clock::now().time_since_epoch().count()));
	std::uniform_real_distribution<float> distributionColor(0.0f, 1.0f);
	v1.normal = DirectX::XMFLOAT3{ 0.0f, 0.0f, -1.0f };

	Vertex v2 = {};
	v2.pos = DirectX::XMFLOAT3{ -0.5f, 0.5f, 0.0f };
	v2.normal = DirectX::XMFLOAT3{ 0.0f, 0.0f, -1.0f };

	Vertex v3 = {};
	v3.pos = DirectX::XMFLOAT3{ 0.5f, 0.5f, 0.0f };
	v3.normal = DirectX::XMFLOAT3{ 0.0f, 0.0f, -1.0f };

	Vertex v4 = {};
	v4.pos = DirectX::XMFLOAT3{ 0.5f, -0.5f, 0.0f };
	v4.normal = DirectX::XMFLOAT3{ 0.0f, 0.0f, -1.0f };

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);

	//First tri
	indices.push_back(0u);
	indices.push_back(1u);
	indices.push_back(2u);
	//Second tri
	indices.push_back(0u);
	indices.push_back(2u);
	indices.push_back(3u);

	m_Meshes.push_back(std::make_unique<Mesh>(vertices, indices));
}

void Model::LoadModel() noexcept
{
	Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile(m_Name, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	DBG_ASSERT(pScene, "Error! Could not read .obj file.");

	ProcessNode(pScene->mRootNode, pScene);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene) noexcept
{
	for (uint32_t i{ 0u }; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessMesh(mesh);
	}

	for (uint32_t i{ 0u }; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

void Model::ProcessMesh(aiMesh* mesh)
{
	std::vector<Vertex> vertices = {};
	std::vector<uint32_t> indices = {};

	for (uint32_t i{ 0u }; i < mesh->mNumVertices; i++)
	{
		Vertex vertex = {};
		vertex.pos.x = mesh->mVertices[i].x;
		vertex.pos.y = mesh->mVertices[i].y;
		vertex.pos.z = mesh->mVertices[i].z;
		
		/*
		Handle textures.
		if (mesh->mTextureCoords[0])
		{
			
		}
		*/

		if (mesh->HasNormals())
		{
			vertex.normal.x = mesh->mNormals[i].x;
			vertex.normal.y = mesh->mNormals[i].y;
			vertex.normal.z = mesh->mNormals[i].z;
		}
		else
		{
			vertex.normal.x = 0.0f;
			vertex.normal.y = 0.0f;
			vertex.normal.z = 0.0f;
		}

		vertices.push_back(vertex);
	}

	for (uint32_t i{ 0u }; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (uint32_t j{ 0u }; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}
	m_Meshes.push_back(std::make_unique<Mesh>(vertices, indices));
}