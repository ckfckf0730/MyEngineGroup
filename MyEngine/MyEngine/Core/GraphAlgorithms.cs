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
    }
}
