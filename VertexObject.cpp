#include "pch.h"
#include "VertexObject.h"

void VertexObject::Initialize(
	std::shared_ptr<Model> objectModel,
	DirectX::XMVECTOR pos,
	DirectX::XMVECTOR rot,
	float scale,
	UpdateType updateType,
	DirectX::XMFLOAT4 color
)
{
	m_pModel = objectModel;
	m_UpdateType = updateType;
	m_Scale = scale;
	m_Color = color;

	using t_clock = std::chrono::high_resolution_clock;
	std::default_random_engine generator(static_cast<UINT>(t_clock::now().time_since_epoch().count()));
	std::uniform_real_distribution<float> distributionColor(0.0f, 180.0f);
	m_ScaleAngle = distributionColor(generator);

	DirectX::XMFLOAT4 temp;
	DirectX::XMStoreFloat4(&temp, pos);
	m_OriginalX = temp.x;

	DirectX::XMMATRIX tempPosMatrix = {};
	tempPosMatrix = DirectX::XMMatrixTranslationFromVector(pos);
	DirectX::XMMATRIX tempRotMatrix = {};
	tempRotMatrix = DirectX::XMMatrixRotationRollPitchYawFromVector(rot);
	DirectX::XMMATRIX tempScaleMatrix = {};
	tempScaleMatrix = DirectX::XMMatrixScalingFromVector(DirectX::XMVectorSet(scale, scale, scale, 1.0f));
	DirectX::XMStoreFloat4x4(&m_Transform, tempScaleMatrix * tempRotMatrix * tempPosMatrix);
	SetConstantBufferView(std::move(MemoryManager::Get().CreateConstantBuffer("Transforms", "ShaderBindables", "TransformsRange", sizeof(DirectX::XMFLOAT4X4))));
}

void VertexObject::Update(float deltaTime)
{
	static const float speed = 1.0f;

	auto m = DirectX::XMLoadFloat4x4(&m_Transform);

	DirectX::XMVECTOR scale;
	DirectX::XMVECTOR rotationQuat;
	DirectX::XMVECTOR translation;
	DirectX::XMMatrixDecompose(&scale, &rotationQuat, &translation, m);
	DirectX::XMMATRIX rotMatrix = DirectX::XMMatrixRotationQuaternion(rotationQuat);
	DirectX::XMFLOAT3 scaleF;
	DirectX::XMStoreFloat3(&scaleF, scale);
	DirectX::XMFLOAT3 rotF;
	DirectX::XMStoreFloat3(&rotF, rotationQuat);
	DirectX::XMFLOAT3 translationF;
	DirectX::XMStoreFloat3(&translationF, translation);

	switch (m_UpdateType)
	{
	case SPIN:
	{
		DirectX::XMVECTOR globalYAxis = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	
		rotMatrix *= DirectX::XMMatrixRotationAxis(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), speed * deltaTime);
		break;
	}
	case RESIZE:
	{
		m_ScaleAngle += speed * deltaTime * 70;
		if (m_ScaleAngle > 180.0f)
		{
			m_ScaleAngle -= 180.0f;
		}
		float val = static_cast<float>((sin(m_ScaleAngle * (M_PI / 180.0f)) * 0.5f) + 0.75f);
		scaleF.x = m_Scale * val;
		scaleF.y = m_Scale * val;
		scaleF.z = m_Scale * val;
		break;
	}
	case MOVEBACKANDFORTH:
	{
		m_ScaleAngle += speed * deltaTime * 70;
		if (m_ScaleAngle > 360.0f)
		{
			m_ScaleAngle -= 360.0f;
		}
		float val = static_cast<float>(sin(m_ScaleAngle * (M_PI / 180.0f)));
		translationF.x = m_OriginalX + (val * 20.0f);
		break;
	}
	default:
	{
		break;
	}
	}
	m = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&scaleF)) * rotMatrix * DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&translationF));

	DirectX::XMFLOAT4X4 tempTransform;
	DirectX::XMStoreFloat4x4(&tempTransform, m);
	SetTransform(tempTransform);

	m = DirectX::XMMatrixTranspose(m);
	MemoryManager::Get().UpdateConstantBuffer(m_TransformConstantBufferView, &m, sizeof(DirectX::XMFLOAT4X4));
}
