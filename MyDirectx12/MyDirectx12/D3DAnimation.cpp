#include"PMDActor.h"
#include"D3DResourceManage.h"
#include <algorithm>
#include<array>
using namespace DirectX;

constexpr float epsilon = 0.0005f;


D3DAnimation* D3DAnimation::LoadVMDFile(const char* fullFilePath)
{
	auto iter = D3DResourceManage::Instance().AnimationTable.find(fullFilePath);
	if (iter != D3DResourceManage::Instance().AnimationTable.end())
	{
		return iter->second;
	}

	FILE* fp;
	errno_t err = fopen_s(&fp, fullFilePath, "rb");
	if (err != 0)
	{
		PrintDebug(L"Load VMD motion file fault.");
		return nullptr;
	}

	D3DAnimation* animation = new D3DAnimation();
	PrintDebug("AnimationTable Insert:");
	PrintDebug(fullFilePath);

	D3DResourceManage::Instance().AnimationTable.insert(
		std::pair<const char*, D3DAnimation*>(fullFilePath, animation));

	fseek(fp, 50, SEEK_SET);

	//-----------motion data----------------
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

	//-----------morph data----------------
	uint32_t morphCount = 0;
	fread(&morphCount, sizeof(morphCount), 1, fp);
	animation->m_morphs.resize(morphCount);
	fread(animation->m_morphs.data(), sizeof(VMDMorph), morphCount, fp);

	//-----------camera animation data----------------
	uint32_t vmdCameraCount = 0;
	fread(&vmdCameraCount, sizeof(vmdCameraCount), 1, fp);
	animation->m_cameraData.resize(vmdCameraCount);
	fread(animation->m_cameraData.data(), sizeof(VMDCamera), vmdCameraCount, fp);

	//-----------light data----------------
	uint32_t vmdLightCount = 0;
	fread(&vmdLightCount, sizeof(vmdLightCount), 1, fp);
	animation->m_lights.resize(vmdLightCount);
	fread(animation->m_lights.data(), sizeof(VMDLight), vmdLightCount, fp);
	
	//-----------self shadow data----------------
	uint32_t selfShadowCount = 0;
	fread(&selfShadowCount, sizeof(selfShadowCount), 1, fp);
	animation->m_selfShadowData.resize(selfShadowCount);
	fread(animation->m_selfShadowData.data(), sizeof(VMDSelfShadow), selfShadowCount, fp);

	//-----------IK enable data----------------
	uint32_t ikSwitchCount = 0;
	fread(&ikSwitchCount, sizeof(ikSwitchCount), 1, fp);
	animation->m_ikEnableData.resize(ikSwitchCount);
	for (auto& ikEnable : animation->m_ikEnableData)
	{
		fread(&ikEnable.frameNo, sizeof(ikEnable.frameNo), 1, fp);

		uint8_t visibleFlg = 0;
		fread(&visibleFlg, sizeof(visibleFlg), 1, fp);

		uint32_t ikBoneCount = 0;
		fread(&ikBoneCount, sizeof(ikBoneCount), 1, fp);

		for (int i = 0; i < ikBoneCount; i++)
		{
			char ikBoneName[20];
			fread(ikBoneName, _countof(ikBoneName), 1, fp);

			uint8_t flg = 0;
			fread(&flg, sizeof(flg), 1, fp);
			ikEnable.ikEnableTable[ikBoneName] = flg;
		}
	}
	

	fclose(fp);

	for (auto& vmdMotion : vmdMotionData)
	{
		auto quaternion = XMLoadFloat4(&vmdMotion.quaternion);
		animation->m_motionData[vmdMotion.boneName].emplace_back(
			KeyFrame(vmdMotion.frameNo, 
				quaternion, vmdMotion.location,
				XMFLOAT2((float)vmdMotion.bezier[3]/127.0f, (float)vmdMotion.bezier[7]/127.0f),
				XMFLOAT2((float)vmdMotion.bezier[11] / 127.0f, (float)vmdMotion.bezier[15] / 127.0f)));
		animation->m_duration = std::max<unsigned int>(animation->m_duration, vmdMotion.frameNo);
	}

	for (auto& motion : animation->m_motionData)
	{
		std::sort(motion.second.begin(), motion.second.end(),
			[](const KeyFrame& lval, const KeyFrame& rval)
			{
				return lval.frameNo <= rval.frameNo;
			});
	}

	animation->m_fileName = fullFilePath;

	return animation;
}

void D3DAnimationInstance::StartAnimation()
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


void D3DAnimationInstance::UpdateAnimation()
{
	DWORD elapsedTime = timeGetTime() - m_startTime;
	unsigned int frameNo = 30 * (elapsedTime / 1000.0f);

	if (frameNo > m_animation->Duration())
	{
		m_startTime = timeGetTime();
		frameNo = 0;
	}

	std::fill(m_owner->m_boneMatrices.begin(),
		m_owner->m_boneMatrices.end(), XMMatrixIdentity());

	for (auto& boneMotion : m_animation->m_motionData)
	{
		auto nodeIter = m_owner->Model()->m_boneNodeTable.find(boneMotion.first);
		if (nodeIter == m_owner->Model()->m_boneNodeTable.end())
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
	RecursiveMatrixMultiply(
		&m_owner->Model()->m_boneNodeTable[m_owner->Model()->m_rootNodeStr], XMMatrixIdentity());

	IKSolve(frameNo);

	std::copy(m_owner->m_boneMatrices.begin(), m_owner->m_boneMatrices.end(), m_owner->m_mapMatrices + 1);
}

//"motion/pose.vmd"
void D3DAnimation::LoadAnimation(const char* path)
{
	LoadVMDFile(path);
}

void D3DAnimationInstance::IKSolve(int frameNo)
{
	auto it = find_if(m_animation->m_ikEnableData.rbegin(), m_animation->m_ikEnableData.rend(),
		[frameNo](const VMDIKEnable& ikenable)
		{
			return ikenable.frameNo <= frameNo;
		});

	for (auto& ik : m_owner->Model()->m_ikData)
	{
		auto ikEnableIt = it->ikEnableTable.find(m_owner->Model()->m_boneNameArr[ik.boneIdx]);
		if (ikEnableIt != it->ikEnableTable.end())
		{
			if (!ikEnableIt->second)
			{
				continue;;
			}
		}

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

void D3DAnimationInstance::SolveLookAt(const PMDIK& ik)
{
	auto rootNode = m_owner->Model()->m_boneNodeAddressArr[ik.nodeIdxes[0]];
	auto targetNode = m_owner->Model()->m_boneNodeAddressArr[ik.targetIdx];
	
	auto rpos1 = XMLoadFloat3(&rootNode->startPos);
	auto tpos1 = XMLoadFloat3(&targetNode->startPos);

	auto rpos2 = XMVector3TransformCoord(
		rpos1, m_owner->m_boneMatrices[ik.nodeIdxes[0]]);
	auto tpos2 = XMVector3TransformCoord(
		tpos1, m_owner->m_boneMatrices[ik.boneIdx]);

	auto originVec = XMVectorSubtract(tpos1, rpos1);
	auto targetVec = XMVectorSubtract(tpos2, rpos2);

	originVec = XMVector3Normalize(originVec);
	targetVec = XMVector3Normalize(targetVec);

	auto up = XMFLOAT3(0, 1, 0);
	auto right = XMFLOAT3(1, 0, 0);
	m_owner->m_boneMatrices[ik.nodeIdxes[0]] = LookAtMatrix(
		originVec, targetVec, up, right);
}

bool IsZeroVector(const DirectX::XMVECTOR& vec)
{
	DirectX::XMVECTOR zeroVec = DirectX::XMVectorZero();
	return DirectX::XMVector3Equal(vec, zeroVec); 
}

void D3DAnimationInstance::SolveCosineIK(const PMDIK& ik)
{
	std::vector<XMVECTOR> positions;

	std::array<float, 2> edgeLens;

	auto& targetNode = m_owner->Model()->m_boneNodeAddressArr[ik.boneIdx];
	auto targetPos = XMVector3Transform(XMLoadFloat3(&targetNode->startPos),
		m_owner->m_boneMatrices[ik.boneIdx]);

	auto endNode = m_owner->Model()->m_boneNodeAddressArr[ik.boneIdx];
	positions.emplace_back(XMLoadFloat3(&endNode->startPos));

	for (auto& chainBoneIdx : ik.nodeIdxes)
	{
		auto boneNode = m_owner->Model()->m_boneNodeAddressArr[chainBoneIdx];
		positions.emplace_back(XMLoadFloat3(&boneNode->startPos));
	}

	reverse(positions.begin(), positions.end());  //make positions' order from root to end 

	edgeLens[0] = XMVector3Length(
		XMVectorSubtract(positions[1], positions[0])).m128_f32[0];
	edgeLens[1] = XMVector3Length(
		XMVectorSubtract(positions[2], positions[1])).m128_f32[0];

	positions[0] = XMVector3Transform(positions[0], m_owner->m_boneMatrices[ik.nodeIdxes[1]]);

	positions[2] = XMVector3Transform(positions[2], m_owner->m_boneMatrices[ik.boneIdx]);

	auto linearVec = XMVectorSubtract(positions[2], positions[0]);

	float A = XMVector3Length(linearVec).m128_f32[0];
	float B = edgeLens[0];
	float C = edgeLens[1];

	linearVec = XMVector3Normalize(linearVec);

	float theta1 = acosf((A * A + B * B - C * C) / (2 * A * B));
	float theta2 = acosf((B * B + C * C - A * A) / (2 * B * C));

	//get the axis, if there is a knee in nodes, then get a fixed axis
	XMVECTOR axis;
	if (std::find(m_owner->Model()->m_kneeIdxes.begin(), m_owner->Model()->m_kneeIdxes.end(), ik.nodeIdxes[0])
		== m_owner->Model()->m_kneeIdxes.end())             //can't find knee node in ik node list
	{
		auto vm = XMVector3Normalize(
			XMVectorSubtract(positions[2], positions[0]));
		auto vt = XMVector3Normalize(
			XMVectorSubtract(targetPos, positions[0]));
		axis = XMVector3Cross(vt, vm);
	}
	else   //the knee arithmetic is a special one, a common IK animation may be not like this
	{
		auto right = XMFLOAT3(1, 0, 0);
		axis = XMLoadFloat3(&right);
	}

	bool isAxisZero = IsZeroVector(axis);

	auto mat1 = XMMatrixTranslationFromVector(-positions[0]);
	if (!isAxisZero)
	{
		mat1 *= XMMatrixRotationAxis(axis, theta1);
	}
	mat1 *= XMMatrixTranslationFromVector(positions[0]);

	auto mat2 = XMMatrixTranslationFromVector(-positions[1]);
	if (!isAxisZero)
	{
		mat2 *= XMMatrixRotationAxis(axis, theta2 - XM_PI);
	}
	mat2 *= XMMatrixTranslationFromVector(positions[1]);

	m_owner->m_boneMatrices[ik.nodeIdxes[1]] *= mat1;
	m_owner->m_boneMatrices[ik.nodeIdxes[0]] = mat2 * m_owner->m_boneMatrices[ik.nodeIdxes[1]];
	m_owner->m_boneMatrices[ik.targetIdx] = m_owner->m_boneMatrices[ik.nodeIdxes[0]];
}

void D3DAnimationInstance::SolveCCDIK(const PMDIK& ik)
{
	std::vector<XMVECTOR> positions;

	auto targetBoneNode = m_owner->Model()->m_boneNodeAddressArr[ik.boneIdx];
	auto targetOriginPos = XMLoadFloat3(&targetBoneNode->startPos);

	auto parentMat = m_owner->m_boneMatrices[m_owner->Model()->m_boneNodeAddressArr[ik.boneIdx]->ikParentBone];
	XMVECTOR det;
	auto invPatrentMat = XMMatrixInverse(&det, parentMat);
	auto targetNextPos = XMVector3Transform(
		targetOriginPos, m_owner->m_boneMatrices[ik.boneIdx] * invPatrentMat);

	std::vector<XMVECTOR> bonePositions;
	auto endPos = XMLoadFloat3(
		&m_owner->Model()->m_boneNodeAddressArr[ik.targetIdx]->startPos);

	for (auto& cidx : ik.nodeIdxes)
	{
		bonePositions.push_back(
			XMLoadFloat3(&m_owner->Model()->m_boneNodeAddressArr[cidx]->startPos));
	}

	std::vector<XMMATRIX> mats(bonePositions.size());
	fill(mats.begin(), mats.end(), XMMatrixIdentity());

	auto ikLimit = ik.limit * XM_PI;

	//root for the times of ik data setting
	for (int c = 0; c < ik.iterations; c++)
	{
		//if target and end are very closed, break
		if (XMVector3Length(
			XMVectorSubtract(endPos, targetNextPos)).m128_f32[0] <= epsilon)
		{
			break;
		}

		for (int bidx = 0; bidx < bonePositions.size(); bidx++)
		{
			const auto& pos = bonePositions[bidx];

			auto vecToEnd = XMVectorSubtract(endPos, pos);
			auto vecToTarget = XMVectorSubtract(targetNextPos, pos);

			vecToEnd = XMVector3Normalize(vecToEnd);
			vecToTarget = XMVector3Normalize(vecToTarget);

			if (XMVector3Length(
				XMVectorSubtract(vecToEnd, vecToTarget)).m128_f32[0] <= epsilon)
			{
				continue;
			}

			auto cross = XMVector3Normalize(XMVector3Cross(vecToEnd, vecToTarget));
			float angle = XMVector3AngleBetweenVectors(vecToEnd, vecToTarget).m128_f32[0];

			angle = min(angle, ikLimit);

			XMMATRIX rot = XMMatrixRotationAxis(cross, angle);
				
			XMMATRIX mat = XMMatrixTranslationFromVector(-pos)
				* rot * XMMatrixTranslationFromVector(pos);

			mats[bidx] *= mat;

			for (auto idx = bidx - 1; idx >= 0; idx--)
			{
				bonePositions[idx] = XMVector3Transform(bonePositions[idx], mat);
			}

			endPos = XMVector3Transform(endPos, mat);

			if (XMVector3Length(
				XMVectorSubtract(endPos, targetNextPos)).m128_f32[0] <= epsilon)
			{
				break;
			}
		}
	}
	
	int idx = 0;
	for (auto& cidx : ik.nodeIdxes)
	{
		m_owner->m_boneMatrices[cidx] = mats[idx];
		idx++;
	}

	auto rootNode = m_owner->Model()->m_boneNodeAddressArr[ik.nodeIdxes.back()];
	RecursiveMatrixMultiply(rootNode, parentMat);

}

void D3DAnimationInstance::RecursiveMatrixMultiply(BoneNode* node, const DirectX::XMMATRIX& mat)
{
	m_owner->m_boneMatrices[node->boneIdx] *= mat;
	for (auto& cnode : node->children)
	{
		RecursiveMatrixMultiply(cnode, m_owner->m_boneMatrices[node->boneIdx]);
	}
}

