using CkfEngine.Core;
using System;
using System.Collections.Generic;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;
using static CkfEngine.Core.EngineObject;

namespace CkfEngine.Core
{
    internal class AnimationControl
    {
        const float epsilon = 0.0005f;
        private uint m_startTime = 0;

        private VMDAnimation m_animation;
        private PMDModelInstance m_owner;

        public AnimationControl(VMDAnimation animation, PMDModelInstance owner)
        {
            m_animation = animation;
            m_owner = owner;
        }

        public void SetModelInstance(PMDModelInstance ins)
        {
            m_owner = ins;
        }

        internal void StartAnimation()
        {
            m_startTime = (uint)TimeManager.GetRunTime();

            UpdateAnimation();
        }

        private static float GetYFromXOnBezier(float x, Vector2 a, Vector2 b, byte n)
        {
            if (a.X == a.Y && b.X == b.Y)
            {
                return x;
            }

            float t = x;
            float k0 = 1 + 3 * a.X - 3 * b.X;
            float k1 = 3 * b.X - 6 * a.X;
            float k2 = 3 * a.X;

            for (int i = 0; i < n; i++)
            {
                float ft = k0 * t * t * t + k1 * t * t + k2 * t - x;

                if (ft <= epsilon && ft >= -epsilon)
                {
                    break;
                }

                t -= ft / 2;
            }
            float r = 1 - t;
            return t * t * t + 3 * t * t * r * b.Y + 3 * t * r * r * a.Y;
        }


        internal void UpdateAnimation()
        {
            uint elapsedTime = (uint)TimeManager.GetRunTime() - m_startTime;
            uint frameNo = (uint)(30 * (elapsedTime / 1000.0f));

            if (frameNo > m_animation.m_duration)
            {
                m_startTime = (uint)TimeManager.GetRunTime();
                frameNo = 0;
            }

            for (int i = 0; i < m_owner.m_boneMatrices.Length; i++)
            {
                m_owner.m_boneMatrices[i] = Matrix4x4.Identity;
            }

            foreach (var boneMotion in m_animation.m_motionData)
            {
                BoneNode nodeIter;

                bool isGet = m_owner.PMDModel.m_boneNodeTable.TryGetValue(boneMotion.Key, out nodeIter);
                if (!isGet)
                {
                    //Console.WriteLine("Can't find bone name:");
                    //Console.WriteLine(boneMotion.Key);
                    continue;
                }
                var motions = boneMotion.Value;
                var reIter = motions.FindLastIndex((motion) => { return motion.frameNo <= frameNo; });

                if (reIter == -1)
                {
                    continue;
                }
                int iter = reIter + 1;
                Matrix4x4 rotation;
                if (iter >= 0 && iter < motions.Count)
                {
                    var t = (float)(frameNo - motions[reIter].frameNo) /
                        (float)(motions[iter].frameNo - motions[reIter].frameNo);
                    t = GetYFromXOnBezier(t, motions[iter].p1, motions[iter].p2, 12);

                    rotation = Matrix4x4.CreateFromQuaternion(
                        Quaternion.Slerp(motions[reIter].quaternion, motions[iter].quaternion, t));
                }
                else
                {
                    rotation = Matrix4x4.CreateFromQuaternion(motions[reIter].quaternion);
                }

                var pos = nodeIter.startPos;
                Matrix4x4 mat = Matrix4x4.CreateTranslation(-pos.X, -pos.Y, -pos.Z) *
                    rotation *
                     Matrix4x4.CreateTranslation(pos.X, pos.Y, pos.Z);
                m_owner.m_boneMatrices[nodeIter.boneIdx] = mat;
            }
            RecursiveMatrixMultiply(m_owner.PMDModel.m_boneNodeTable[m_owner.PMDModel.m_rootNodeStr], Matrix4x4.Identity);

            IKSolve((int)frameNo);

            D3DAPICall.UpdatePMDBoneMatrices(m_owner.m_uid, m_owner.m_boneMatrices, m_owner.m_boneMatrices.Length);
        }

        private void RecursiveMatrixMultiply(BoneNode node, Matrix4x4 mat)
        {
            m_owner.m_boneMatrices[node.boneIdx] *= mat;
            if (node.children == null)
            {
                node.children = new List<BoneNode>();
            }

            foreach (var cnode in node.children)
            {
                RecursiveMatrixMultiply(cnode, m_owner.m_boneMatrices[node.boneIdx]);
            }
        }

        void IKSolve(int frameNo)
        {
            int it = 0;
            for (int i = m_animation.m_ikEnableData.Length - 1; i >= 0; i--)
            {
                if (m_animation.m_ikEnableData[i].frameNo <= frameNo)
                {
                    it = i;
                    break;
                }
            }

            foreach (var ik in m_owner.PMDModel.m_iks)
            {
                bool ikEnableIt;
                bool isGet = m_animation.m_ikEnableData[it].ikEnableTable.TryGetValue(m_owner.PMDModel.m_boneNameArr[ik.boneIdx], out ikEnableIt);
                if (isGet)
                {
                    if (!ikEnableIt)
                    {
                        continue; ;
                    }
                }

                var childrenNodesCount = ik.nodeIdxes.Count;

                switch (childrenNodesCount)
                {
                    case 0:
                        throw new Exception("childrenNodesCount can't be 0!");
                    case 1:
                        SolveLookAt(ik);
                        break;
                    case 2:
                        SolveCosineIK(ik);
                        break;
                    default:
                        SolveCCDIK(ik);
                        break;
                }
            }
        }

        private void SolveLookAt(PMDIK ik)
        {
            var rootNode = m_owner.PMDModel.m_boneNodeAddressArr[ik.nodeIdxes[0]];
            var targetNode = m_owner.PMDModel.m_boneNodeAddressArr[ik.targetIdx];

            Vector3 rpos1 = rootNode.startPos;
            Vector3 tpos1 = targetNode.startPos;

            Vector3 rpos2 = Vector3.Transform(rpos1, m_owner.m_boneMatrices[ik.nodeIdxes[0]]);
            Vector3 tpos2 = Vector3.Transform(tpos1, m_owner.m_boneMatrices[ik.boneIdx]);

            Vector3 originVec = Vector3.Subtract(tpos1, rpos1);
            Vector3 targetVec = Vector3.Subtract(tpos2, rpos2);

            originVec = Vector3.Normalize(originVec);
            targetVec = Vector3.Normalize(targetVec);

            var up = new Vector3(0, 1, 0);
            //var right = new Vector3(1, 0, 0);
            m_owner.m_boneMatrices[ik.nodeIdxes[0]] =
                Matrix4x4.CreateLookAt(originVec, targetVec, up);
        }

        private List<Vector3> positions = new List<Vector3>();
        private float[] edgeLens = new float[2];
        void SolveCosineIK(PMDIK ik)
        {
            positions.Clear();

            var targetNode = m_owner.PMDModel.m_boneNodeAddressArr[ik.boneIdx];
            var targetPos = Vector3.Transform(targetNode.startPos, m_owner.m_boneMatrices[ik.boneIdx]);

            var endNode = m_owner.PMDModel.m_boneNodeAddressArr[ik.boneIdx];
            positions.Add(endNode.startPos);

            for (int i = 0; i < ik.nodeIdxes.Count; i++)
            {
                var boneNode = m_owner.PMDModel.m_boneNodeAddressArr[ik.nodeIdxes[i]];
                positions.Add(boneNode.startPos);
            }

            positions.Reverse();  //make positions' order from root to end 

            edgeLens[0] = Vector3.Distance(positions[1], positions[0]);
            edgeLens[1] = Vector3.Distance(positions[2], positions[1]);

            positions[0] = Vector3.Transform(positions[0], m_owner.m_boneMatrices[ik.nodeIdxes[1]]);

            positions[2] = Vector3.Transform(positions[2], m_owner.m_boneMatrices[ik.boneIdx]);

            var linearVec = Vector3.Subtract(positions[2], positions[0]);

            float A = linearVec.Length();
            float B = edgeLens[0];
            float C = edgeLens[1];

            linearVec = Vector3.Normalize(linearVec);

            float theta1 = (float)Math.Acos((A * A + B * B - C * C) / (2 * A * B));
            float theta2 = (float)Math.Acos((B * B + C * C - A * A) / (2 * B * C));

            //get the axis, if there is a knee in nodes, then get a fixed axis
            Vector3 axis;
            if (m_owner.PMDModel.m_kneeIdxes.FindIndex((num) => { return num == ik.nodeIdxes[0]; }) == -1)
            {
                //can't find knee node in ik node list
                var vm = Vector3.Normalize(Vector3.Subtract(positions[2], positions[0]));
                var vt = Vector3.Normalize(Vector3.Subtract(targetPos, positions[0]));
                axis = Vector3.Cross(vt, vm);
            }
            else   //the knee arithmetic is a special one, a common IK animation may be not like this
            {
                axis = new Vector3(1, 0, 0);
            }

            bool isAxisZero = GraphAlgorithms.IsZeroVector(axis);

            var mat1 = Matrix4x4.CreateTranslation(-positions[0]);
            if (!isAxisZero)
            {
                mat1 *= Matrix4x4.CreateFromAxisAngle(axis, theta1);
            }
            mat1 *= Matrix4x4.CreateTranslation(positions[0]);

            var mat2 = Matrix4x4.CreateTranslation(-positions[1]);
            if (!isAxisZero)
            {
                mat2 *= Matrix4x4.CreateFromAxisAngle(axis, theta2 - (float)Math.PI);
            }
            mat2 *= Matrix4x4.CreateTranslation(positions[1]);

            m_owner.m_boneMatrices[ik.nodeIdxes[1]] *= mat1;
            m_owner.m_boneMatrices[ik.nodeIdxes[0]] = mat2 * m_owner.m_boneMatrices[ik.nodeIdxes[1]];
            m_owner.m_boneMatrices[ik.targetIdx] = m_owner.m_boneMatrices[ik.nodeIdxes[0]];
        }

        private List<Vector3> bonePositions = new List<Vector3>();
        private List<Matrix4x4> mats = new List<Matrix4x4>();
        private void SolveCCDIK(PMDIK ik)
        {
            positions.Clear();

            var targetBoneNode = m_owner.PMDModel.m_boneNodeAddressArr[ik.boneIdx];
            var targetOriginPos = targetBoneNode.startPos;

            var parentMat = m_owner.m_boneMatrices[m_owner.PMDModel.m_boneNodeAddressArr[ik.boneIdx].ikParentBone];
            Matrix4x4 invPatrentMat;
            Matrix4x4.Invert(parentMat, out invPatrentMat);
            var targetNextPos = Vector3.Transform(
                targetOriginPos, m_owner.m_boneMatrices[ik.boneIdx] * invPatrentMat);

            bonePositions.Clear();
            var endPos = m_owner.PMDModel.m_boneNodeAddressArr[ik.targetIdx].startPos;

            for (int i =0;i< ik.nodeIdxes.Count;i++)
            {
                bonePositions.Add(m_owner.PMDModel.m_boneNodeAddressArr[ik.nodeIdxes[i]].startPos);
            }

            mats.Clear();
            for(int i =0;i< bonePositions.Count; i++)
            {
                mats.Add(Matrix4x4.Identity);
            }

            var ikLimit = ik.limit * (float)Math.PI;

            //root for the times of ik data setting
            for (int c = 0; c < ik.iterations; c++)
            {
                //if target and end are very closed, break
                if (Vector3.Distance(endPos, targetNextPos) <= epsilon)
                {
                    break;
                }

                for (int bidx = 0; bidx < bonePositions.Count; bidx++)
                {
                    var pos = bonePositions[bidx];

                    var vecToEnd = Vector3.Subtract(endPos, pos);
                    var vecToTarget = Vector3.Subtract(targetNextPos, pos);

                    vecToEnd = Vector3.Normalize(vecToEnd);
                    vecToTarget = Vector3.Normalize(vecToTarget);

                    if (Vector3.Distance(vecToEnd, vecToTarget) <= epsilon)
                    {
                        continue;
                    }

                    var cross = Vector3.Normalize(Vector3.Cross(vecToEnd, vecToTarget));
                    float angle = GraphAlgorithms.AngleBetweenNormalVectors(vecToEnd, vecToTarget);

                    angle = Math.Min(angle, ikLimit);

                    Matrix4x4 rot = Matrix4x4.CreateFromAxisAngle(cross, angle);

                    Matrix4x4 mat = Matrix4x4.CreateTranslation(-pos)
                        * rot * Matrix4x4.CreateTranslation(pos);

                    mats[bidx] *= mat;

                    for (int idx = bidx - 1; idx >= 0; idx--)
                    {
                        bonePositions[idx] =  Vector3.Transform(bonePositions[idx], mat);
                    }

                    endPos = Vector3.Transform(endPos, mat);

                    if (Vector3.Distance(endPos, targetNextPos) <= epsilon)
                    {
                        break;
                    }
                }
            }

            int idx2 = 0;
            for (int cidx =0; cidx<  ik.nodeIdxes.Count;cidx++)
            {
                m_owner.m_boneMatrices[ik.nodeIdxes[cidx]] = mats[idx2];
                idx2++;
            }

            var rootNode = m_owner.PMDModel.m_boneNodeAddressArr[ik.nodeIdxes.Last()];
            RecursiveMatrixMultiply(rootNode, parentMat);

        }
    }



    internal class PMDModelInstance : ModelInstance
    {
        public PMDModelInstance(PMDModel model, ulong uid) : base(model,uid)
        {
            CreateTransformView();
        }

        internal Matrix4x4[] m_boneMatrices;
        internal PMDModel PMDModel
        {
            get { return m_model as PMDModel; }
        }


        internal void CreateTransformView()
        {
            m_boneMatrices = new Matrix4x4[PMDModel.m_bones.Count];
        }
    }

    internal class ModelInstance
    {
        public ModelInstance(Model model, ulong uid)
        {
            m_model = model;
            m_uid = uid;
        }

        internal Model m_model;
        internal ulong m_uid;
    }



    public class VMDAnimation
    {
        public uint m_duration;
        public string m_fileName;

        public VMDMorph[] m_morphs;
        public VMDCamera[] m_cameraData;
        public VMDLight[] m_lights;
        public VMDSelfShadow[] m_selfShadowData;
        public VMDIKEnable[] m_ikEnableData;
        public Dictionary<string, List<KeyFrame>> m_motionData = new Dictionary<string, List<KeyFrame>>();
    }


    public struct VMDMotion
    {
        public string boneName; //lenth 15 bytes 
        public uint frameNo;
        public Vector3 location;
        public Quaternion quaternion;
        public byte[] bezier;  //64 bytes
    }

    public struct VMDMorph
    {
        public string name;  //15 bytes
        public uint frameNo;
        public float wight;
    }
    public struct VMDCamera
    {
        public uint frameNo;
        public float distance;
        public Vector3 pos;
        public Vector3 eulerAngle;
        public byte[] Interpolation; //24 bytes
        public uint fov;
        public byte persFlg;
    }
    public struct VMDLight
    {
        public uint frameNo;
        public Vector3 rgb;
        public Vector3 vec;
    }
    public struct VMDSelfShadow
    {
        public uint frameNo;
        public byte mode;
        public float distance;
    }
    public struct VMDIKEnable
    {
        public uint frameNo;
        public Dictionary<string, bool> ikEnableTable;
    }
    public struct KeyFrame
    {
        public uint frameNo;
        public Quaternion quaternion;
        public Vector3 offset;
        public Vector2 p1, p2;
        KeyFrame(uint fno, ref Quaternion q, ref Vector3 ofst, ref Vector2 ip1, ref Vector2 ip2)
        {
            frameNo = fno;
            quaternion = q;
            offset = ofst;
            p1 = ip1;
            p2 = ip2;
        }

    }
}
