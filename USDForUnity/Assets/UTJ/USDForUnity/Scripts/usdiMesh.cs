using System;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{

    [ExecuteInEditMode]
    public class usdiMesh : usdiXform
    {
        class MeshBuffer
        {
            public Vector3[] positions;
            public Vector3[] normals;
            public Vector2[] uvs;
            public int[] indices;

            public void Allocate(ref usdi.MeshSummary summary, ref usdi.MeshData md)
            {
                {
                    positions = new Vector3[md.num_points];
                    md.points = usdi.GetArrayPtr(positions);
                }
                if (summary.has_normals)
                {
                    normals = new Vector3[md.num_points];
                    md.normals = usdi.GetArrayPtr(normals);
                }
                if (summary.has_uvs)
                {
                    uvs = new Vector2[md.num_points];
                    md.uvs = usdi.GetArrayPtr(uvs);
                }
                {
                    indices = new int[md.num_indices_triangulated];
                    md.indices_triangulated = usdi.GetArrayPtr(indices);
                }
            }

            public void Clear()
            {
                positions = null;
                normals = null;
                uvs = null;
                indices = null;
            }
        }


        #region fields
        usdi.Mesh m_mesh;
        usdi.MeshData m_meshData;
        usdi.MeshSummary m_meshSummary;
        MeshBuffer m_buf = new MeshBuffer();

        Mesh m_umesh;
        bool m_umeshIsEmpty;
        int m_prevVertexCount;
        int m_prevIndexCount;
        int m_frame;

        List<usdiSplitMesh> m_children = new List<usdiSplitMesh>();
        usdi.SplitedMeshData[] m_splitedData;

        // for Unity 5.5 or later
        bool m_directVBUpdate;
        usdi.MapContext m_ctxVB;
        usdi.MapContext m_ctxIB;
        #endregion



        #region properties
        public usdi.Mesh usdiObject { get { return m_mesh; } }
        public usdi.MeshSummary meshSummary { get { return m_meshSummary; } }
        public usdi.MeshData meshData { get { return m_meshData; } }
        public bool directVBUpdate { get { return m_directVBUpdate; } }
        #endregion



        #region impl
        Mesh usdiAddMeshComponents()
        {
            Mesh mesh = null;

            var go = gameObject;
            MeshFilter meshFilter = go.GetComponent<MeshFilter>();

            if (meshFilter == null || meshFilter.sharedMesh == null)
            {
                mesh = new Mesh();

                if (meshFilter == null)
                {
                    meshFilter = go.AddComponent<MeshFilter>();
                }

                meshFilter.sharedMesh = mesh;

                MeshRenderer renderer = go.GetComponent<MeshRenderer>();

                if (renderer == null)
                {
                    renderer = go.AddComponent<MeshRenderer>();
                }

#if UNITY_EDITOR
                Material material = UnityEngine.Object.Instantiate(GetDefaultMaterial());
                material.name = "Material_0";
                renderer.sharedMaterial = material;
#endif
            }
            else
            {
                mesh = meshFilter.sharedMesh;
            }

            mesh.MarkDynamic();
            return mesh;
        }

        usdiSplitMesh usdiAddSplit()
        {
            usdiSplitMesh split;
            if (m_children.Count == 0)
            {
                split = GetOrAddComponent<usdiSplitMesh>();
            }
            else
            {
                var child = new GameObject("Split[" + m_children.Count + "]");
                child.GetComponent<Transform>().SetParent(GetComponent<Transform>(), false);
                split = child.AddComponent<usdiSplitMesh>();
            }
            split.usdiOnLoad(this, m_children.Count);
            m_children.Add(split);
            return split;
        }

        void usdiGatherExistingSplits()
        {
            {
                var child = GetComponent<usdiSplitMesh>();
                if(child != null)
                {
                    m_children.Add(child);
                }
            }

            var t = GetComponent<Transform>();
            for (int i=1; ; ++i)
            {
                var child = t.FindChild("Split[" + i + "]");
                if (child == null) { break; }

                var split = child.GetComponent<usdiSplitMesh>();
                if (child != null) { m_children.Add(split); }
            }
        }

#if UNITY_EDITOR
        static MethodInfo s_GetBuiltinExtraResourcesMethod;
        public static Material GetDefaultMaterial()
        {
            if (s_GetBuiltinExtraResourcesMethod == null)
            {
                BindingFlags bfs = BindingFlags.NonPublic | BindingFlags.Static;
                s_GetBuiltinExtraResourcesMethod = typeof(EditorGUIUtility).GetMethod("GetBuiltinExtraResource", bfs);
            }
            return (Material)s_GetBuiltinExtraResourcesMethod.Invoke(null, new object[] { typeof(Material), "Default-Material.mat" });
        }
#endif

        public override void usdiOnLoad(usdi.Schema schema)
        {
            base.usdiOnLoad(schema);

            m_mesh = usdi.usdiAsMesh(schema);
            if (!m_mesh)
            {
                Debug.LogWarning("schema is not Mesh!");
                return;
            }

            usdi.usdiMeshGetSummary(m_mesh, ref m_meshSummary);
            m_umesh = usdiAddMeshComponents();
            m_umeshIsEmpty = m_umesh.vertexCount == 0;

            usdiGatherExistingSplits();
            for (int i = 0; i < m_children.Count; ++i)
            {
                m_children[i].usdiOnLoad(this, m_children.Count);
            }
        }

        public override void usdiOnUnload()
        {
            base.usdiOnUnload();
            m_mesh = default(usdi.Mesh);
        }


        void usdiAllocateMeshData(double t)
        {
            usdi.MeshData md = default(usdi.MeshData);
            usdi.usdiMeshReadSample(m_mesh, ref md, t, true);

            // skip if already allocated
            if (m_prevVertexCount == md.num_points &&
                m_prevIndexCount == md.num_indices_triangulated)
            {
                return;
            }


            m_meshData = md;
            if (m_meshData.num_splits != 0)
            {
                m_splitedData = new usdi.SplitedMeshData[m_meshData.num_splits];
                m_meshData.splits = usdi.GetArrayPtr(m_splitedData);
                usdi.usdiMeshReadSample(m_mesh, ref m_meshData, t, true);

                while (m_children.Count < m_meshData.num_splits)
                {
                    usdiAddSplit();
                }

                for (int i = 0; i < m_children.Count; ++i)
                {
                    m_children[i].usdiAllocateMeshData(ref m_splitedData[i]);
                }
            }
            else
            {
                m_buf.Allocate(ref m_meshSummary, ref m_meshData);
            }
        }

        void usdiFreeMeshData()
        {
            for (int i = 0; i < m_children.Count; ++i)
            {
                m_children[i].usdiFreeMeshData(ref m_splitedData[i]);
            }

            m_buf.Clear();

            m_meshData.points = IntPtr.Zero;
            m_meshData.normals = IntPtr.Zero;
            m_meshData.uvs = IntPtr.Zero;
            m_meshData.indices_triangulated = IntPtr.Zero;
        }

        void usdiReadMeshData(double t)
        {
            // todo: update heterogenous mesh if possible
            m_directVBUpdate = m_stream.directVBUpdate &&
                m_meshSummary.topology_variance == usdi.TopologyVariance.Homogenous &&
                m_ctxVB.resource != IntPtr.Zero;
            bool copyVertexData = !m_directVBUpdate;

#if UNITY_EDITOR
            if (m_stream.usdForceSingleThread)
            {
                usdi.usdiMeshReadSample(m_mesh, ref m_meshData, t, copyVertexData);
            }
            else
#endif
            {
                usdi.usdiMeshReadSampleAsync(m_mesh, ref m_meshData, t, copyVertexData);
            }


            if (m_directVBUpdate)
            {
                m_meshData.indices_triangulated = IntPtr.Zero;
                usdi.usdiExtQueueVertexBufferUpdateTask(ref m_meshData, ref m_ctxVB, ref m_ctxIB);
            }
        }

        void usdiUpdateMeshData(double t, bool topology, bool close)
        {
            if (!m_directVBUpdate)
            {
                if (m_meshData.num_splits != 0)
                {
                    for (int i = 0; i < m_children.Count; ++i)
                    {
                        m_children[i].usdiUpdateMeshData(topology, close);
                    }
                }
                else
                {
                    m_umesh.vertices = m_buf.positions;
                    if (m_meshSummary.has_normals)
                    {
                        m_umesh.normals = m_buf.normals;
                    }
                    if (m_meshSummary.has_uvs)
                    {
                        m_umesh.uv = m_buf.uvs;
                    }

                    if (topology)
                    {
                        m_umesh.SetIndices(m_buf.indices, MeshTopology.Triangles, 0);

                        if (!m_meshSummary.has_normals)
                        {
                            m_umesh.RecalculateNormals();
                        }
                    }

                    m_umeshIsEmpty = m_umesh.vertexCount == 0;
                    m_umesh.UploadMeshData(close);
#if UNITY_5_5_OR_NEWER
                    if(m_stream.directVBUpdate)
                    {
                        m_ctxVB.resource = m_umesh.GetNativeVertexBufferPtr(0);
                        //m_ctxIB.resource = m_umesh.GetNativeIndexBufferPtr();
                    }
#endif
                }

            }
            m_prevVertexCount = m_meshData.num_points;
            m_prevIndexCount = m_meshData.num_indices_triangulated;
        }

        public override void usdiAsyncUpdate(double time)
        {
            base.usdiAsyncUpdate(time);
            if (!m_needsUpdate) { return; }

            switch (m_meshSummary.topology_variance)
            {
                case usdi.TopologyVariance.Constant:
                    if (m_frame == 0 && m_umeshIsEmpty)
                    {
                        usdiAllocateMeshData(time);
                        usdiReadMeshData(time);
                    }
                    break;

                case usdi.TopologyVariance.Homogenous:
                    if (m_frame == 0)
                    {
                        usdiAllocateMeshData(time);
                    }
                    usdiReadMeshData(time);
                    break;

                case usdi.TopologyVariance.Heterogenous:
                    usdiAllocateMeshData(time);
                    usdiReadMeshData(time);
                    break;
            }
        }

        public override void usdiUpdate(double time)
        {
            if (!m_needsUpdate) { return; }
            base.usdiUpdate(time);

            switch (m_meshSummary.topology_variance) {
                case usdi.TopologyVariance.Constant:
                    if(m_frame == 0 && m_umeshIsEmpty)
                    {
                        usdiUpdateMeshData(time, true, true);
                        usdiFreeMeshData();
                    }
                    break;

                case usdi.TopologyVariance.Homogenous:
                    if (m_frame == 0)
                    {
                        usdiUpdateMeshData(time, true, false);
                    }
                    else
                    {
                        usdiUpdateMeshData(time, false, false);
                    }
                    break;

                case usdi.TopologyVariance.Heterogenous:
                    usdiUpdateMeshData(time, true, false);
                    break;
            }

            ++m_frame;
        }
        #endregion


        #region callbacks
        static int s_nth_OnWillRenderObject = 0;

        void Update()
        {
            s_nth_OnWillRenderObject = 0;
        }

        void OnWillRenderObject()
        {
            ++s_nth_OnWillRenderObject;

            if(s_nth_OnWillRenderObject == 1)
            {
                GL.IssuePluginEvent(usdi.usdiGetRenderEventFunc(), 0);
            }
        }
        #endregion
    }

}
