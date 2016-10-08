using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using UnityEngine;

namespace UTJ
{
    class usdiStreamUpdator
    {
        public struct Config
        {
            Bool forceSingleThread;
            Bool directVBUpdate;
        }

        IntPtr m_rep;

        public usdiStreamUpdator(usdi.Context usd, usdiStream stream) { m_rep = _Ctor(usd, stream); }
        ~usdiStreamUpdator() { _Dtor(m_rep); }

        public void SetConfig(ref Config config) { _SetConfig(m_rep, ref config); }
        public void Add(usdiElement component) { _Add(m_rep, component); }
        public void AsyncUpdate(double time) { _AsyncUpdate(m_rep, time); }
        public void Update(double time) { _Update(m_rep, time); }


        #region internal
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr _Ctor(usdi.Context usd, usdiStream stream);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void _Dtor(IntPtr rep);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void _SetConfig(IntPtr rep, ref Config config);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void _Add(IntPtr rep, usdiElement component);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void _AsyncUpdate(IntPtr rep, double time);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void _Update(IntPtr rep, double time);
        #endregion
    }

}