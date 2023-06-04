#include"D3DEntity.h"
#include"D3DResourceManage.h"
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

	m_vmdMotionData.resize(motionDataNum);
	for (auto& vmdMotion : m_vmdMotionData)
	{
		fread(vmdMotion.boneName, sizeof(vmdMotion.boneName), 1, fp);
		fread(&vmdMotion.frameNo,
			sizeof(vmdMotion.frameNo) + sizeof(vmdMotion.location) +
			sizeof(vmdMotion.quaternion) + sizeof(vmdMotion.bezier),
			1, fp);
	}


	fclose(fp);

	return 1;
}

void D3DAnimation::StartAnimation()
{
	m_startTime = timeGetTime();
	for (auto& vmdMotion : m_vmdMotionData)
	{
		auto quaternion = XMLoadFloat4(&vmdMotion.quaternion);
		m_motionData[vmdMotion.boneName].emplace_back(
			KeyFrame(vmdMotion.frameNo, quaternion));
	}

	UpdateAnimation();
}

void D3DAnimation::UpdateAnimation()
{
	DWORD elapsedTime = timeGetTime() - m_startTime;
	unsigned int frameNo = 30 * (elapsedTime / 1000.0f);

	std::fill(m_owner->m_boneMatrices.begin(),
		m_owner->m_boneMatrices.end(), XMMatrixIdentity());

	for (auto& boneMotion : m_motionData)
	{
		auto nodeIter = m_owner->m_boneNodeTable.find(boneMotion.first);
		if (nodeIter == m_owner->m_boneNodeTable.end())
		{
			PrintDebug("Can't find bone name:");
			PrintDebug(boneMotion.first.c_str());
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

			rotation = XMMatrixRotationQuaternion(reIter->quaternion) * (1 - t) +
				XMMatrixRotationQuaternion(iter->quaternion) * t;
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