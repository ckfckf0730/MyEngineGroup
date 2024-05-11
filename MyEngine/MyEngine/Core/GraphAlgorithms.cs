using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    public static class GraphAlgorithms
    {
        public static Vector3 RotateFrom3Axis(Vector3 vector, Vector3 rot3Axis)
        {
            return Vector3.Transform(vector,
                Matrix4x4.CreateFromAxisAngle(new Vector3(0, 0, 1), rot3Axis.Z) *
                Matrix4x4.CreateFromAxisAngle(new Vector3(1, 0, 0), rot3Axis.X) *
                Matrix4x4.CreateFromAxisAngle(new Vector3(0, 1, 0), rot3Axis.Y));
        }

        public static Matrix4x4 MatRotateFrom3Axis(Vector3 rot3Axis)
        {
            return Matrix4x4.CreateFromAxisAngle(new Vector3(0, 0, 1), rot3Axis.Z) *
                Matrix4x4.CreateFromAxisAngle(new Vector3(1, 0, 0), rot3Axis.X) *
                Matrix4x4.CreateFromAxisAngle(new Vector3(0, 1, 0), rot3Axis.Y);
        }

        public static bool IsZeroVector(Vector3 vec)
        {
            return Vector3.Equals(vec, Vector3.Zero);
        }

        public static float AngleBetweenVectors(Vector3 vector1, Vector3 vector2)
        {
            float dotProduct = Vector3.Dot(vector1, vector2);

            float length1 = vector1.Length();
            float length2 = vector2.Length();

            float cosAngle = dotProduct / (length1 * length2);

            float angleRadians = (float)Math.Acos(cosAngle);

            return angleRadians;
        }

        public static float AngleBetweenNormalVectors(Vector3 vector1, Vector3 vector2)
        {
            float dotProduct = Vector3.Dot(vector1, vector2);

            float angleRadians = (float)Math.Acos(dotProduct);

            return angleRadians;
        }
    }
}
