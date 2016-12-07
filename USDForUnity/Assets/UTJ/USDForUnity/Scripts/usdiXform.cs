using System;
using UnityEngine;

namespace UTJ
{

    [Serializable]
    public class usdiXform : usdiSchema
    {
        #region fields
        [SerializeField] protected Transform m_trans;

        usdi.Xform m_xf;
        usdi.XformData m_xfData = usdi.XformData.default_value;
        protected usdi.UpdateFlags m_updateFlags;
        #endregion

        #region properties
        public usdi.Xform nativeXformPtr
        {
            get { return m_xf; }
        }
        #endregion

        #region impl
        protected override usdiIElement usdiSetupSchemaComponent()
        {
            return GetOrAddComponent<usdiXformElement>();
        }

        public override void usdiOnLoad()
        {
            base.usdiOnLoad();
            m_xf = usdi.usdiAsXform(m_schema);
            m_trans = GetComponent<Transform>();
        }

        public override void usdiOnUnload()
        {
            base.usdiOnUnload();
            m_xf = default(usdi.Xform);
        }

        public override void usdiAsyncUpdate(double time)
        {
            m_updateFlags = usdi.usdiPrimGetUpdateFlags(m_xf);
            usdi.usdiXformReadSample(m_xf, ref m_xfData, time);
        }

        public override void usdiUpdate(double time)
        {
            base.usdiUpdate(time);
            if(m_goAssigned)
            {
                usdi.usdiUniTransformAssign(m_trans, ref m_xfData);
            }

            //// fall back
            //if ((m_xfData.flags & (int)usdi.XformData.Flags.UpdatedPosition) != 0)
            //{
            //    m_trans.localPosition = m_xfData.position;
            //}
            //if ((m_xfData.flags & (int)usdi.XformData.Flags.UpdatedRotation) != 0)
            //{
            //    m_trans.localRotation = m_xfData.rotation;
            //}
            //if ((m_xfData.flags & (int)usdi.XformData.Flags.UpdatedScale) != 0)
            //{
            //    m_trans.localScale = m_xfData.scale;
            //}
        }
        #endregion
    }

}
