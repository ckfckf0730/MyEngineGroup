#include"D3DEntity.h"
#include"D3DResourceManage.h"
#include <algorithm>
using namespace DirectX;

int D3DAnimation::LoadVMDFile(const char* fullFilePath, D3DModel* owner)
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
				quaternion,
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

	std::fill(m_owner->m_boneMatrices.begin(),
		m_owner->m_boneMatrices.end(), XMMatrixIdentity());

	for (auto& boneMotion : m_motionData)
	{
		auto nodeIter = m_owner->m_boneNodeTable.find(boneMotion.first);
		if (nodeIter == m_owner->m_boneNodeTable.end())
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
		m_owner->m_boneMatrices[nodeIter->second.boneIdx] = mat;
	}
	m_owner->RecursiveMatrixMultiply(
		&m_owner->m_boneNodeTable[m_owner->m_rootNodeStr], XMMatrixIdentity());

	std::copy(m_owner->m_boneMatrices.begin(), m_owner->m_boneMatrices.end(), m_owner->m_mapMatrices + 1);
}