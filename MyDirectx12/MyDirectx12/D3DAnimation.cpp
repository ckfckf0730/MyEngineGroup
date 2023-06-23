#include"PMDActor.h"
#include"D3DResourceManage.h"
#include <algorithm>
#include<array>
using namespace DirectX;

int D3DAnimation::LoadVMDFile(const char* fullFilePath, PMDModel* owner)
{
	m_owner = owner;
	FILE* fp;
	errno_t err = fopen_s(&fp, fullFilePath, "rb");
	if (err != 0)
	{
		PrintDebug(L"Load VMD motion file fault.");
		return -1;
	}

	fseek(fp, 50, SEEK_SET);

	unsigned int motionDataNum = 0;
	fread(&motionDataNum, sizeof(motionDataNum), 1, fp);

	std::vector<VMDMotion> vmdMotionData(motionDataNum);
	for (auto& vmdMotion : vmdMotionData)
	{
		fread(vmdMotion.boneName, sizeof(vmdMotion.boneName), 1, fp);
		fread(&vmdMotion.frameNo,
			sizeof(vmdMotion.frameNo) + sizeof(vmdMotion.location) +
			sizeof(vmdMotion.quaternion) + sizeof(vmdMotion.bezier),
			1, fp);
	}


	fclose(fp);

	for (auto& vmdMotion : vmdMotionData)
	{
		auto quaternion = XMLoadFloat4(&vmdMotion.quaternion);
		m_motionData[vmdMotion.boneName].emplace_back(
			KeyFrame(vmdMotion.frameNo, 
				quaternion, vmdMotion.location,
				XMFLOAT2((float)vmdMotion.bezier[3]/127.0f, (float)vmdMotion.bezier[7]/127.0f),
				XMFLOAT2((float)vmdMotion.bezier[11] / 127.0f, (float)vmdMotion.bezier[15] / 127.0f)));
		m_duration = std::max<unsigned int>(m_duration, vmdMotion.frameNo);
	}

	for (auto& motion : m_motionData)
	{
		std::sort(motion.second.begin(), motion.second.end(),
			[](const KeyFrame& lval, const KeyFrame& rval)
			{
				return lval.frameNo <= rval.frameNo;
			});
	}

	return 1;
}

void D3DAnimation::StartAnimation()
{
	m_startTime = timeGetTime();
	

	UpdateAnimation();
}

float GetYFromXOnBezier(float x, const XMFLOAT2& a, const XMFLOAT2& b, uint8_t n)
{
	if (a.x == a.y && b.x == b.y)
	{
		return x;
	}

	float t = x;
	const float k0 = 1 + 3 * a.x - 3 * b.x;
	const float k1 = 3 * b.x - 6 * a.x;
	const float k2 = 3 * a.x;

	constexpr float epsilon = 0.0005f;

	for (int i = 0; i < n; i++)
	{
		auto ft = k0 * t * t * t + k1 * t * t + k2 * t - x;

		if (ft <= epsilon && ft >= -epsilon)
		{
			break;
		}

		t -= ft / 2;
	}
	auto r = 1 - t;
	return t * t * t + 3 * t * t * r * b.y + 3 * t * r * r * a.y;
}


void D3DAnimation::UpdateAnimation()
{
	DWORD elapsedTime = timeGetTime() - m_startTime;
	unsigned int frameNo = 30 * (elapsedTime / 1000.0f);

	if (frameNo > m_duration)
	{
		m_startTime = timeGetTime();
		frameNo = 0;
	}

	std::fill(m_boneMatrices.begin(),
		m_boneMatrices.end(), XMMatrixIdentity());

	for (auto& boneMotion : m_motionData)
	{
		auto nodeIter = m_boneNodeTable.find(boneMotion.first);
		if (nodeIter == m_boneNodeTable.end())
		{
			/*PrintDebug("Can't find bone name:");
			PrintDebug(boneMotion.first.c_str());*/
			continue;
		}
		auto motions = boneMotion.second;
		auto reIter = std::find_if(motions.rbegin(), motions.rend(),
			[frameNo](const KeyFrame& motion)
			{
				return motion.frameNo <= frameNo;
			});
		if (reIter == motions.rend())
		{
			continue;
		}
		auto iter = reIter.base();
		XMMATRIX rotation;
		if (iter != motions.end())
		{
			auto t = static_cast<float>(frameNo - reIter->frameNo) /
				static_cast<float>(iter->frameNo - reIter->frameNo);
			t = GetYFromXOnBezier(t, iter->p1, iter->p2, 12);

			rotation = XMMatrixRotationQuaternion(
				XMQuaternionSlerp(reIter->quaternion, iter->quaternion, t));
		}
		else
		{
			rotation = XMMatrixRotationQuaternion(reIter->quaternion);
		}

		auto& pos = nodeIter->second.startPos;
		auto mat = XMMatrixTranslation(-pos.x, -pos.y, -pos.z) *
			rotation *
			XMMatrixTranslation(pos.x, pos.y, pos.z);
		m_boneMatrices[nodeIter->second.boneIdx] = mat;
	}
	RecursiveMatrixMultiply(
		&m_boneNodeTable[m_rootNodeStr], XMMatrixIdentity());

	std::copy(m_boneMatrices.begin(), m_boneMatrices.end(), m_owner->m_mapMatrices + 1);
}



void D3DAnimation::IKSolve() 
{
	for (auto& ik : m_ikData)
	{
		auto childrenNodesCount = ik.nodeIdxes.size();

		switch (childrenNodesCount)
		{
		case 0:
			assert(0);
			continue;
		case 1:
			SolveLookAt(ik);
			break;
		case 2:
			SolveCosineIK(ik);
			break;
		default:
			SolveCCDIK(ik);
		}
	}
}

XMMATRIX LookAtMatrix(const XMVECTOR& lookat, XMFLOAT3& up, XMFLOAT3& right)
{
	XMVECTOR vz = lookat;
	XMVECTOR vy = XMVector3Normalize(XMLoadFloat3(&up));

	XMVECTOR vx = XMVector3Normalize(XMVector3Cross(vy, vz));
	vy = XMVector3Normalize(XMVector3Cross(vz, vx));

	if (std::abs(XMVector3Dot(vy, vz).m128_f32[0]) == 1.0f)
	{
		vx = XMVector3Normalize(XMLoadFloat3(&right));
		vy = XMVector3Normalize(XMVector3Cross(vz, vx));
		vx = XMVector3Normalize(XMVector3Cross(vy, vz));
	}

	XMMATRIX ret = XMMatrixIdentity();
	ret.r[0] = vx;
	ret.r[1] = vy;
	ret.r[2] = vz;
	return ret;
}

XMMATRIX LookAtMatrix(const XMVECTOR& origin, const XMVECTOR& lookat,
	XMFLOAT3& up, XMFLOAT3& right)
{
	return XMMatrixTranspose(LookAtMatrix(origin, up, right)) *
		LookAtMatrix(lookat, up, right);
}

void D3DAnimation::SolveLookAt(const PMDIK& ik)
{
	auto rootNode = m_boneNodeAddressArr[ik.nodeIdxes[0]];
	auto targetNode = m_boneNodeAddressArr[ik.targetIdx];
	
	auto rpos1 = XMLoadFloat3(&rootNode->startPos);
	auto tpos1 = XMLoadFloat3(&targetNode->startPos);

	auto rpos2 = XMVector3TransformCoord(
		rpos1,m_boneMatrices[ik.nodeIdxes[0]]);
	auto tpos2 = XMVector3TransformCoord(
		tpos1, m_boneMatrices[ik.boneIdx]);

	auto originVec = XMVectorSubtract(tpos1, rpos1);
	auto targetVec = XMVectorSubtract(tpos2, rpos2);

	originVec = XMVector3Normalize(originVec);
	targetVec = XMVector3Normalize(targetVec);

	auto up = XMFLOAT3(0, 1, 0);
	auto right = XMFLOAT3(1, 0, 0);
	m_boneMatrices[ik.nodeIdxes[0]] = LookAtMatrix(
		originVec, targetVec, up, right);
}

void D3DAnimation::SolveCCDIK(const PMDIK& ik)
{

}
void D3DAnimation::SolveCosineIK(const PMDIK& ik)
{
	std::vector<XMVECTOR> positions;

	std::array<float, 2> edgeLens;

	auto& targetNode = m_boneNodeAddressArr[ik.boneIdx];
	auto targetPos = XMVector3Transform(XMLoadFloat3(&targetNode->startPos),
		m_boneMatrices[ik.boneIdx]);

	auto endNode = m_boneNodeAddressArr[ik.boneIdx];
	positions.emplace_back(XMLoadFloat3(&endNode->startPos));

	for (auto& chainBoneIdx : ik.nodeIdxes)
	{
		auto boneNode = m_boneNodeAddressArr[chainBoneIdx];
		positions.emplace_back(XMLoadFloat3(&boneNode->startPos));
	}

	reverse(positions.begin(), positions.end());

	edgeLens[0] = XMVector3Length(
		XMVectorSubtract(positions[1], positions[0])).m128_f32[0];
	edgeLens[1] = XMVector3Length(
		XMVectorSubtract(positions[2], positions[1])).m128_f32[0];

	positions[0] = XMVector3Transform(positions[0], m_boneMatrices[ik.nodeIdxes[1]]);

	positions[2] = XMVector3Transform(positions[2], m_boneMatrices[ik.boneIdx]);

	auto linearVec = XMVectorSubtract(positions[2], positions[0]);

	float A = XMVector3Length(linearVec).m128_f32[0];
	float B = edgeLens[0];
	float C = edgeLens[1];

	linearVec = XMVector3Normalize(linearVec);

	float theta1 = acosf((A * A + B * B - C * C) / (2 * A * B));
	float theta2 = acosf((B * B + C * C - A * A) / (2 * B * C));

	
}

void D3DAnimation::RecursiveMatrixMultiply(BoneNode* node, const DirectX::XMMATRIX& mat)
{
	m_boneMatrices[node->boneIdx] *= mat;
	for (auto& cnode : node->children)
	{
		RecursiveMatrixMultiply(cnode, m_boneMatrices[node->boneIdx]);
	}
}

