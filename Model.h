#pragma once
#include "Mesh.h"

class Model
{
public:
	Model() noexcept = default;
	~Model() noexcept = default;

	void Initialize(const std::string path) noexcept;

public:
	const std::string& GetName() const noexcept {
		return m_Name;
	}

	const std::vector<std::unique_ptr<Mesh>>& GetMeshes() noexcept { return m_Meshes; }
private:
	void LoadTri() noexcept;
	void LoadRec() noexcept;
	void LoadModel() noexcept;
	void ProcessNode(aiNode* node, const aiScene* scene) noexcept;
	void ProcessMesh(aiMesh* mesh);
private:

	std::string m_Name = "";

	std::vector<std::unique_ptr<Mesh>> m_Meshes = {};
};