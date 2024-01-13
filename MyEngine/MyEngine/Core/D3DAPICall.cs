using Microsoft.Win32.SafeHandles;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine
{
    internal class D3DAPICall
    {
        #region  dll function

        [DllImport("MyDirectx12.dll")]
        static extern int InitD3d(System.IntPtr hwnd);

        [DllImport("MyDirectx12.dll")]
        public static extern int CreateRenderTarget(System.IntPtr hwnd, 
            ulong uid, uint width, uint height);

        [DllImport("MyDirectx12.dll")]
        public static extern int Render(ulong uid);

   
        [DllImport("MyDirectx12.dll")]
        public static extern int SetPMDModel(ulong uid,string fileFullName);

        [DllImport("MyDirectx12.dll")]
        public static extern int SetBasicModel(ulong uid, string fileFullName);

        [DllImport("MyDirectx12.dll")]
        public static extern void SetModelTransform(ulong uid, Matrix4x4 matrix);

        [DllImport("MyDirectx12.dll")]
        public static extern int LoadAnimation(ulong uid, string path);

        [DllImport("MyDirectx12.dll")]
        public static extern void UpdateAnimation(ulong uid);

        [DllImport("MyDirectx12.dll", CharSet = CharSet.Auto)]
        public static extern void TryGetLog( ref int isGet, StringBuilder msg);

        [DllImport("MyDirectx12.dll")]
        public static extern void SetCameraTransform(
            Vector3 eye, Vector3 target, Vector3 up);

        [DllImport("MyDirectx12.dll")]
        public static extern void SetCameraProjection(
            float FovAngleY, float AspectRatio, float NearZ, float Far);

        [DllImport("MyDirectx12.dll")]
        public static extern int SetPmdStandardPipeline();

        #endregion



        public struct Vertex
        {
            public Vector3 pos;
            public Vector2 uv;

            public Vertex(Vector3 _pos, Vector2 _uv)
            {
                pos = _pos;
                uv = _uv;
            }
        }

        public int Init(IntPtr hwnd)
        {
            WinConsole.Initialize();


            if(InitD3d(hwnd) < 1)
            {
                Console.WriteLine("InitD3d fault");
                return -1;
            }

            if (SetPmdStandardPipeline() < 1)
            {
                Console.WriteLine("Create standard pipeline fault");
                return -1;
            }

            
            return 1;
        }

        public void ClickOnce()
        {
            //Render();
        }


    }

    static class WinConsole
    {
        static public void Initialize(bool alwaysCreateNewConsole = true)
        {
            bool consoleAttached = true;
            if (alwaysCreateNewConsole
                || (AttachConsole(ATTACH_PARRENT) == 0
                && Marshal.GetLastWin32Error() != ERROR_ACCESS_DENIED))
            {
                consoleAttached = AllocConsole() != 0;
            }

            if (consoleAttached)
            {
                InitializeOutStream();
                InitializeInStream();
            }
            Console.OutputEncoding = Encoding.GetEncoding("utf-8");
        }

        private static void InitializeOutStream()
        {
            var fs = CreateFileStream("CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, FileAccess.Write);
            if (fs != null)
            {
                var writer = new StreamWriter(fs) { AutoFlush = true };
                Console.SetOut(writer);
                Console.SetError(writer);
            }
        }

        private static void InitializeInStream()
        {
            var fs = CreateFileStream("CONIN$", GENERIC_READ, FILE_SHARE_READ, FileAccess.Read);
            if (fs != null)
            {
                Console.SetIn(new StreamReader(fs));
            }
        }

        private static FileStream CreateFileStream(string name, uint win32DesiredAccess, uint win32ShareMode,
                                FileAccess dotNetFileAccess)
        {
            var file = new SafeFileHandle(CreateFileW(name, win32DesiredAccess, win32ShareMode, IntPtr.Zero, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, IntPtr.Zero), true);
            if (!file.IsInvalid)
            {
                var fs = new FileStream(file, dotNetFileAccess);
                return fs;
            }
            return null;
        }

        #region Win API Functions and Constants
        [DllImport("kernel32.dll",
            EntryPoint = "AllocConsole",
            SetLastError = true,
            CharSet = CharSet.Auto,
            CallingConvention = CallingConvention.StdCall)]
        private static extern int AllocConsole();

        [DllImport("kernel32.dll",
            EntryPoint = "AttachConsole",
            SetLastError = true,
            CharSet = CharSet.Auto,
            CallingConvention = CallingConvention.StdCall)]
        private static extern UInt32 AttachConsole(UInt32 dwProcessId);

        [DllImport("kernel32.dll",
            EntryPoint = "CreateFileW",
            SetLastError = true,
            CharSet = CharSet.Auto,
            CallingConvention = CallingConvention.StdCall)]
        private static extern IntPtr CreateFileW(
              string lpFileName,
              UInt32 dwDesiredAccess,
              UInt32 dwShareMode,
              IntPtr lpSecurityAttributes,
              UInt32 dwCreationDisposition,
              UInt32 dwFlagsAndAttributes,
              IntPtr hTemplateFile
            );

        private const UInt32 GENERIC_WRITE = 0x40000000;
        private const UInt32 GENERIC_READ = 0x80000000;
        private const UInt32 FILE_SHARE_READ = 0x00000001;
        private const UInt32 FILE_SHARE_WRITE = 0x00000002;
        private const UInt32 OPEN_EXISTING = 0x00000003;
        private const UInt32 FILE_ATTRIBUTE_NORMAL = 0x80;
        private const UInt32 ERROR_ACCESS_DENIED = 5;

        private const UInt32 ATTACH_PARRENT = 0xFFFFFFFF;

        #endregion
    }
}
