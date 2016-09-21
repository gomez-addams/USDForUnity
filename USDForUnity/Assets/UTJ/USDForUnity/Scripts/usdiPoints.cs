using System;
using UnityEngine;

namespace UTJ
{

    public class usdiPoints : usdiXform
    {
        #region fields
        usdi.Points     m_points;
        usdi.PointsSummary m_summary = default(usdi.PointsSummary);
        usdi.PointsData m_pointsData;
        usdi.Attribute  m_attrRotations;

        Vector3[] m_positions;
        Vector3[] m_velocities;
        Vector4[] m_rotations;

        IntPtr m_ptrRotations;
        #endregion


        #region properties
        public Vector3[] positions { get { return m_positions; } }
        public Vector3[] velocities { get { return m_velocities; } }
        #endregion


        #region impl
        public override void usdiOnLoad(usdi.Schema schema)
        {
            base.usdiOnLoad(schema);

            m_points = usdi.usdiAsPoints(schema);
            if(!m_points)
            {
                Debug.LogWarning("schema is not Points!");
                return;
            }
            usdi.usdiPointsGetSummary(m_points, ref m_summary);
            m_attrRotations = usdi.usdiFindAttribute(m_points, "rotations");
        }

        public override void usdiOnUnload()
        {
            base.usdiOnUnload();

            m_points = default(usdi.Points);
            m_summary = default(usdi.PointsSummary);
            m_pointsData = default(usdi.PointsData);
            m_attrRotations = default(usdi.Attribute);

            m_positions = null;
            m_velocities = null;
            m_rotations = null;

            m_ptrRotations = default(IntPtr);
        }

        public override void usdiAsyncUpdate(double time)
        {
            base.usdiAsyncUpdate(time);
            if (!m_needsUpdate) { return; }

            usdi.PointsData tmp = usdi.PointsData.default_value;
            usdi.usdiPointsReadSample(m_points, ref tmp, time, true);

            // allocate points data
            if (m_pointsData.num_points == tmp.num_points)
            {
                // no need to allocate
            }
            else
            {
                m_pointsData.num_points = tmp.num_points;

                m_positions = new Vector3[m_pointsData.num_points];
                m_pointsData.points = usdi.GetArrayPtr(m_positions);

                if (m_summary.has_velocities)
                {
                    m_velocities = new Vector3[m_pointsData.num_points];
                    m_pointsData.velocities = usdi.GetArrayPtr(m_velocities);
                }
                if (m_attrRotations)
                {
                    m_rotations = new Vector4[m_pointsData.num_points];
                    m_ptrRotations = usdi.GetArrayPtr(m_rotations);
                }
            }

            // read points data
            if (m_pointsData.num_points > 0)
            {
                usdi.usdiPointsReadSampleAsync(m_points, ref m_pointsData, time, true);
                if (m_attrRotations)
                {
                    usdi.usdiAttrReadArraySample(m_attrRotations, m_ptrRotations, m_rotations.Length, time);
                }
            }
        }

        public override void usdiUpdate(double time)
        {
            if (!m_needsUpdate) { return; }
            base.usdiUpdate(time);
        }
        #endregion
    }

}
