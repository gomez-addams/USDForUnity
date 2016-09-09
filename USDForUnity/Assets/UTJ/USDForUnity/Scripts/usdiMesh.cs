using System;
using System.Runtime.InteropServices;
using System.Reflection;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{

    public class usdiMesh : usdiXform
    {
        usdi.Mesh m_mesh;
        usdi.MeshData m_meshData;
        usdi.MeshSummary m_meshSummary;

        Mesh m_umesh;
        Vector3[] m_positions;
        Vector3[] m_velocities;
        Vector3[] m_normals;
        Vector2[] m_uvs;
        int[] m_indices;




        Mesh usdiAddMeshComponents()
        {
            Mesh mesh = null;

            var go = gameObject;
            MeshFilter meshFilter = go.GetComponent<MeshFilter>();

            if (meshFilter == null || meshFilter.sharedMesh == null)
            {
                mesh = new Mesh();
                mesh.MarkDynamic();

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

            return mesh;
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
        }

        public override void usdiOnUnload()
        {
            base.usdiOnUnload();
            m_mesh = default(usdi.Mesh);
        }


        void usdiAllocateMeshData(double t)
        {
            usdi.MeshData md = default(usdi.MeshData);
            usdi.usdiMeshReadSample(m_mesh, ref md, t);

            if( m_meshData.num_points == md.num_points &&
                m_meshData.num_indices_triangulated == md.num_indices_triangulated)
            {
                // skip allocation
                return;
            }

            m_meshData = md;
            {
                m_positions = new Vector3[m_meshData.num_points];
                m_meshData.points = usdi.GetArrayPtr(m_positions);
            }
            if (m_meshSummary.has_normals)
            {
                m_normals = new Vector3[m_meshData.num_points];
                m_meshData.normals = usdi.GetArrayPtr(m_normals);
            }
            if (m_meshSummary.has_uvs)
            {
                m_uvs = new Vector2[m_meshData.num_points];
                m_meshData.uvs = usdi.GetArrayPtr(m_uvs);
            }
            {
                m_indices = new int[m_meshData.num_indices_triangulated];
                m_meshData.indices_triangulated = usdi.GetArrayPtr(m_indices);
            }

        }

        void usdiUpdateMeshData(double t, bool topology, bool close)
        {
            if (usdi.usdiMeshReadSample(m_mesh, ref m_meshData, t))
            {
                m_umesh.vertices = m_positions;
                if (m_meshSummary.has_normals)
                {
                    m_umesh.normals = m_normals;
                }
                if (m_meshSummary.has_uvs)
                {
                    m_umesh.uv = m_uvs;
                }

                if(topology)
                {
                    m_umesh.SetIndices(m_indices, MeshTopology.Triangles, 0);

                    if (!m_meshSummary.has_normals)
                    {
                        m_umesh.RecalculateNormals();
                    }
                }

                m_umesh.UploadMeshData(false);
            }
        }

        public override void usdiUpdate(double time)
        {
            base.usdiUpdate(time);

            if(!m_mesh) { return; }

            switch (m_meshSummary.topology_variance) {
                case usdi.TopologyVariance.Constant:
                    if(m_meshData.points == IntPtr.Zero)
                    {
                        usdiAllocateMeshData(time);
                        usdiUpdateMeshData(time, true, true);
                    }
                    break;

                case usdi.TopologyVariance.Homogenous:
                    if (m_meshData.points == IntPtr.Zero)
                    {
                        usdiAllocateMeshData(time);
                        usdiUpdateMeshData(time, true, false);
                    }
                    else
                    {
                        usdiUpdateMeshData(time, false, false);
                    }
                    break;

                case usdi.TopologyVariance.Heterogenous:
                    usdiAllocateMeshData(time);
                    usdiUpdateMeshData(time, true, false);
                    break;
            }
        }
    }

}
