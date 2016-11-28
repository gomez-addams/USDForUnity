using System;
using System.Reflection;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{

    [Serializable]
    public class usdiSubmesh
    {
        #region fields
        [SerializeField] usdiStream m_stream;
        [SerializeField] int m_nth;
        [SerializeField] Mesh m_umesh;

        [SerializeField] Transform m_trans; // null in master nodes
        [SerializeField] MeshFilter m_meshFilter; // null in master nodes
        [SerializeField] MeshRenderer m_renderer; // null in master nodes

        usdi.Schema m_schema;
        bool m_setupRequierd = true;
        Vector3[] m_points;
        Vector3[] m_normals;
        Vector2[] m_uvs;
        int[] m_indices;

        // for Unity 5.5 or later
        IntPtr m_VB, m_IB;
        usdi.VertexUpdateCommand m_vuCmd;
        #endregion


        #region properties
        public Mesh mesh { get { return m_umesh; } }
        #endregion


        #region impl
        public void usdiSetupMesh()
        {
            if (!m_setupRequierd) { return; }
            m_setupRequierd = false;

            if (m_umesh == null)
            {
                m_umesh = new Mesh();
                m_umesh.MarkDynamic();
            }
        }

        public void usdiSetupComponents(usdiMesh parent_mesh)
        {
            if (!m_setupRequierd) { return; }
            m_setupRequierd = false;

            GameObject go;
            if(m_nth == 0)
            {
                go = parent_mesh.gameObject;
            }
            else
            {
                string name = "Submesh[" + m_nth + "]";
                var parent = parent_mesh.GetComponent<Transform>();
                var child = parent.FindChild(name);
                if (child != null)
                {
                    go = child.gameObject;
                }
                else
                {
                    go = new GameObject(name);
                    go.GetComponent<Transform>().SetParent(parent_mesh.GetComponent<Transform>(), false);
                }
            }

            m_trans = go.GetComponent<Transform>();
            m_meshFilter = go.GetComponent<MeshFilter>();
            if (m_meshFilter == null || m_meshFilter.sharedMesh == null)
            {
                m_umesh = new Mesh();
                if (m_meshFilter == null)
                {
                    m_meshFilter = go.AddComponent<MeshFilter>();
                }
                m_meshFilter.sharedMesh = m_umesh;
            }
            else
            {
                m_umesh = m_meshFilter.sharedMesh;
            }
            m_umesh.MarkDynamic();

            m_renderer = go.GetComponent<MeshRenderer>();
            if (m_renderer == null)
            {
                m_renderer = go.AddComponent<MeshRenderer>();
#if UNITY_EDITOR
                Material material = UnityEngine.Object.Instantiate(GetDefaultMaterial());
                material.name = "Material_0";
                m_renderer.sharedMaterial = material;
#endif
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


        public void usdiAllocateMeshData(ref usdi.MeshSummary summary,  ref usdi.MeshData meshData)
        {
            {
                m_points = new Vector3[meshData.num_points];
                meshData.points = usdi.GetArrayPtr(m_points);
            }
            {
                m_normals = new Vector3[meshData.num_points];
                meshData.normals = usdi.GetArrayPtr(m_normals);
            }
            if (summary.has_uvs)
            {
                m_uvs = new Vector2[meshData.num_points];
                meshData.uvs = usdi.GetArrayPtr(m_uvs);
            }
            {
                m_indices = new int[meshData.num_indices_triangulated];
                meshData.indices_triangulated = usdi.GetArrayPtr(m_indices);
            }
        }
        public void usdiAllocateMeshData(ref usdi.MeshSummary summary, ref usdi.SubmeshData[] submeshData)
        {
            var data = submeshData[m_nth];
            {
                m_points = new Vector3[data.num_points];
                data.points = usdi.GetArrayPtr(m_points);
            }
            {
                m_normals = new Vector3[data.num_points];
                data.normals = usdi.GetArrayPtr(m_normals);
            }
            if (summary.has_uvs)
            {
                m_uvs = new Vector2[data.num_points];
                data.uvs = usdi.GetArrayPtr(m_uvs);
            }
            {
                m_indices = new int[data.num_points];
                data.indices = usdi.GetArrayPtr(m_indices);
            }
            submeshData[m_nth] = data;
        }

        public void usdiFreeMeshData()
        {
            m_points = null;
            m_normals = null;
            m_uvs = null;
            m_indices = null;
        }

        public void usdiKickVBUpdateTask(ref usdi.MeshData data)
        {
            bool active = m_renderer.isVisible;
            if(active)
            {
                if (m_vuCmd == null)
                {
                    m_vuCmd = new usdi.VertexUpdateCommand(usdi.usdiPrimGetNameS(m_schema));
                }
                m_vuCmd.Update(ref data, m_VB, IntPtr.Zero);
            }
        }
        public void usdiKickVBUpdateTask(ref usdi.SubmeshData data)
        {
            bool active = m_renderer.isVisible;
            if (active)
            {
                if (m_vuCmd == null)
                {
                    m_vuCmd = new usdi.VertexUpdateCommand(usdi.usdiPrimGetNameS(m_schema));
                }
                m_vuCmd.Update(ref data, m_VB, IntPtr.Zero);
            }
        }

        public void usdiUpdateBounds(ref usdi.MeshData data)
        {
            usdi.usdiUniMeshAssignBounds(m_umesh, ref data.center, ref data.extents);
        }
        public void usdiUpdateBounds(ref usdi.SubmeshData data)
        {
            usdi.usdiUniMeshAssignBounds(m_umesh, ref data.center, ref data.extents);
        }

        public void usdiSetActive(bool v)
        {
            if(m_trans != null)
            {
                m_trans.gameObject.SetActive(v);
            }
        }

        public void usdiOnLoad(usdiMesh parent, int nth)
        {
            m_stream = parent.stream;
            m_schema = parent.nativeSchemaPtr;
            m_nth = nth;
        }

        public void usdiOnUnload()
        {
            usdiFreeMeshData();
        }


        public void usdiUploadMeshData(bool directVBUpdate, bool topology, bool close)
        {
            if (directVBUpdate && m_VB != IntPtr.Zero)
            {
                // nothing to do here
            }
            else
            {
                m_umesh.vertices = m_points;
                if (m_normals != null) { m_umesh.normals = m_normals; }
                if (m_uvs != null) { m_umesh.uv = m_uvs; }

                if (topology)
                {
                    m_umesh.SetIndices(m_indices, MeshTopology.Triangles, 0);
                    if (m_normals == null) { m_umesh.RecalculateNormals(); }
                }

                //m_umesh.UploadMeshData(close);
                m_umesh.UploadMeshData(false);
#if UNITY_5_5_OR_NEWER
                if (m_stream.directVBUpdate)
                {
                    m_VB = m_umesh.GetNativeVertexBufferPtr(0);
                    m_IB = m_umesh.GetNativeIndexBufferPtr();
                }
#endif
            }
        }
    }
    #endregion

}
