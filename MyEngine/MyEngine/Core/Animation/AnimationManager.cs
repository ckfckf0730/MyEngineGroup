using CkfEngine.Core;
using System;
using System.Collections.Generic;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using static CkfEngine.Core.EngineObject;

namespace CkfEngine.Core
{
    internal class AnimationControl
    {
        const float epsilon = 0.0005f;
        private uint m_startTime = 0;

        private VMDAnimation m_animation;
        private PMDModelInstance m_owner;

        public AnimationControl( VMDAnimation animation, PMDModelInstance owner)
        {
            m_animation = animation;
            m_owner = owner;
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

                bool isGet = m_owner.m_model.m_boneNodeTable.TryGetValue(boneMotion.Key, out nodeIter);
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
            RecursiveMatrixMultiply(m_owner.m_model.m_boneNodeTable[m_owner.m_model.m_rootNodeStr], Matrix4x4.Identity);

            //    IKSolve(frameNo);

            D3DAPICall.UpdatePMDBoneMatrices(m_owner.m_uid, m_owner.m_boneMatrices, m_owner.m_boneMatrices.Length);
        }

        private void RecursiveMatrixMultiply(BoneNode node, Matrix4x4 mat)
        {
            m_owner.m_boneMatrices[node.boneIdx] *= mat;
            if(node.children == null)
            {
                node.children = new List<BoneNode>();
            }

            foreach (var cnode in node.children)
            {
                RecursiveMatrixMultiply(cnode, m_owner.m_boneMatrices[node.boneIdx]);
            }
        }

    }

    internal class PMDModelInstance
    {
        public PMDModelInstance(PMDModel model,ulong uid)
        {
            m_model = model;
            m_uid = uid;
            CreateTransformView();
        }

        internal Matrix4x4[] m_boneMatrices;
        internal PMDModel m_model;
        internal ulong m_uid;

        internal void CreateTransformView()
        {
            m_boneMatrices = new Matrix4x4[m_model.m_bones.Count];
        }
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
