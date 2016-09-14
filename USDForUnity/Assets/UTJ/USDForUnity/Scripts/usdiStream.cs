using System;
using System.Collections.Generic;
using System.Threading;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{
    [Serializable]
    public class usdiImportOptions
    {
        public usdi.InterpolationType interpolation = usdi.InterpolationType.Linear;
        public float scale = 1.0f;
        public bool swapHandedness = true;
        public bool swapFaces = true;
    }


    [ExecuteInEditMode]
    public class usdiStream : MonoBehaviour
    {
        public string m_path;
        public usdiImportOptions m_importOptions = new usdiImportOptions();
        public double m_time = 0.0;
        public double m_timeScale = 1.0;

        [Header("Debug")]
#if UNITY_EDITOR
        public bool m_forceSingleThread = false;
        public bool m_detailedLog = false;
        bool m_isCompiling = false;
#endif
        public bool m_directVBUpdate = true;
        int m_taskQueue;

        usdi.Context m_ctx;
        List<usdiElement> m_elements = new List<usdiElement>();
        double m_prevUpdateTime = Double.NaN;
        ManualResetEvent m_eventAsyncUpdate = new ManualResetEvent(true);


        public bool directVBUpdate { get { return m_directVBUpdate; } }
        public int taskQueue { get { return m_taskQueue; } }


        void usdiLog(string message)
        {
#if UNITY_EDITOR
            if (m_detailedLog)
            {
                Debug.Log(message);
            }
#endif
        }

        public static usdiElement usdiCreateNode(Transform parent, usdi.Schema schema)
        {
            {
                var name = usdi.S(usdi.usdiGetName(schema));
                var child = parent.FindChild(name);
                if (child != null)
                {
                    return child.GetComponent<usdiElement>();
                }
            }

            GameObject go = null;
            usdiElement elem = null;

            if (go == null)
            {
                var points = usdi.usdiAsPoints(schema);
                if (points)
                {
                    go = new GameObject();
                    elem = go.AddComponent<usdiPoints>();
                }
            }
            if (go == null)
            {
                var mesh = usdi.usdiAsMesh(schema);
                if(mesh)
                {
                    go = new GameObject();
                    elem = go.AddComponent<usdiMesh>();
                }
            }
            if (go == null)
            {
                var cam = usdi.usdiAsCamera(schema);
                if (cam)
                {
                    go = new GameObject();
                    elem = go.AddComponent<usdiCamera>();
                }
            }
            if (go == null)
            {
                var xf = usdi.usdiAsXform(schema);
                if (xf)
                {
                    go = new GameObject();
                    elem = go.AddComponent<usdiXform>();
                }
            }

            if(go != null)
            {
                go.GetComponent<Transform>().SetParent(parent);
                go.name = usdi.S(usdi.usdiGetName(schema));
            }

            return elem;
        }

        public void usdiCreateNodeRecursive(Transform parent, usdi.Schema schema, Action<usdiElement> node_handler)
        {
            if(!schema) { return; }

            var elem = usdiCreateNode(parent, schema);
            if (elem != null )
            {
                elem.stream = this;
                elem.usdiOnLoad(schema);
                if (node_handler != null) { node_handler(elem); }
            }

            var trans = elem == null ? parent : elem.GetComponent<Transform>();
            int num_children = usdi.usdiGetNumChildren(schema);
            for(int ci = 0; ci < num_children; ++ci)
            {
                var child = usdi.usdiGetChild(schema, ci);
                usdiCreateNodeRecursive(trans, child, node_handler);
            }
        }

        public void usdiApplyImportConfig()
        {
            usdi.ImportConfig conf;
            conf.interpolation = m_importOptions.interpolation;
            conf.scale = m_importOptions.scale;
            conf.triangulate = true;
            conf.swap_handedness = m_importOptions.swapHandedness;
            conf.swap_faces = m_importOptions.swapFaces;
            usdi.usdiSetImportConfig(m_ctx, ref conf);
        }

        public bool usdiLoad(string path)
        {
            usdiUnload();

            m_path = path;
            m_ctx = usdi.usdiCreateContext();
            usdiApplyImportConfig();
            if (!usdi.usdiOpen(m_ctx, Application.streamingAssetsPath + "/" + m_path))
            {
                usdi.usdiDestroyContext(m_ctx);
                m_ctx = default(usdi.Context);
                usdiLog("usdiStream: failed to load " + m_path);
                return false;
            }

            usdiCreateNodeRecursive(GetComponent<Transform>(), usdi.usdiGetRoot(m_ctx),
                (e) => { m_elements.Add(e); });

            usdiAsyncUpdate(0.0);
            usdiUpdate(0.0);

            m_taskQueue = usdi.usdiExtCreateTaskQueue();
            usdiLog("usdiStream: loaded " + m_path);
            return true;
        }

        public void usdiUnload()
        {
            if(m_ctx)
            {
                foreach (var e in m_elements)
                {
                    e.usdiOnUnload();
                }
                usdi.usdiDestroyContext(m_ctx);
                m_ctx = default(usdi.Context);
                usdi.usdiExtDestroyTaskQueue(m_taskQueue);
                m_taskQueue = 0;
                usdiLog("usdiStream: unloaded " + m_path);
            }
        }

        public void usdiAsyncUpdate(double t)
        {
            // skip if update is not needed
            if (t == m_prevUpdateTime) { return; }

            usdiApplyImportConfig();

            // update all elements
            foreach (var e in m_elements)
            {
                e.usdiAsyncUpdate(t);
            }

            //// task parallel async update.
            //// this will be faster if m_elements is large enough. but otherwise slower.
            //{
            //    int division = 16;
            //    int numActiveTasks = 0;
            //    int granurarity = Mathf.Max(m_elements.Count / division, 1);
            //    int numTasks = m_elements.Count / granurarity + (m_elements.Count % granurarity == 0 ? 0 : 1);
            //    for (int ti=0; ti< numTasks; ++ti)
            //    {
            //        Interlocked.Increment(ref numActiveTasks);
            //        ThreadPool.QueueUserWorkItem((object state) =>
            //        {
            //            int nth = (int)state;
            //            try
            //            {
            //                int begin = granurarity * nth;
            //                int end = Mathf.Min(granurarity * (nth + 1), m_elements.Count);
            //                for (int i = begin; i < end; ++i)
            //                {
            //                    m_elements[i].usdiAsyncUpdate(t);
            //                }
            //            }
            //            finally
            //            {
            //                Interlocked.Decrement(ref numActiveTasks);
            //            }
            //        }, ti);
            //    }
            //    while(numActiveTasks > 0) { }
            //}
        }

        public void usdiUpdate(double t)
        {
            if (t == m_prevUpdateTime) { return; }

            // update all elements
            foreach (var e in m_elements)
            {
                e.usdiUpdate(t);
            }

            // kick VB update task
            if(m_directVBUpdate)
            {
                GL.IssuePluginEvent(usdi.usdiGetRenderEventFunc(), m_taskQueue);
            }

            m_prevUpdateTime = t;
        }

        void Awake()
        {
            usdi.InitializePlugin();
        }

        void Start()
        {
            usdiLoad(m_path);
        }

#if UNITY_EDITOR
        void OnDisable()
        {
            if (!EditorApplication.isPlaying && EditorApplication.isPlayingOrWillChangePlaymode)
            {
                usdiUnload();
            }
        }
#endif

        void OnDestroy()
        {
            usdiUnload();
        }

        void OnApplicationQuit()
        {
            usdiUnload();
        }

        void Update()
        {
#if UNITY_EDITOR
            if (EditorApplication.isCompiling && !m_isCompiling)
            {
                m_isCompiling = true;
                usdiUnload();
            }
            else if(!EditorApplication.isCompiling && m_isCompiling)
            {
                m_isCompiling = false;
                usdiLoad(m_path);
            }
#endif

            // make sure all previous tasks are finished
            usdi.usdiExtFlushTaskQueue(m_taskQueue);

            // kick async update tasks
#if UNITY_EDITOR
            if (m_forceSingleThread)
            {
                usdiAsyncUpdate(m_time);
            }
            else
#endif
            {
                m_eventAsyncUpdate.Reset();
                ThreadPool.QueueUserWorkItem((object state) =>
                {
                    try
                    {
                        usdiAsyncUpdate(m_time);
                    }
                    finally
                    {
                        m_eventAsyncUpdate.Set();
                    }
                });
            }
        }

        void LateUpdate()
        {
            // wait usdiAsyncUpdate() to complete
            m_eventAsyncUpdate.WaitOne();

            usdiUpdate(m_time);

#if UNITY_EDITOR
            if(Application.isPlaying)
#endif
            {
                m_time += Time.deltaTime * m_timeScale;
            }
        }
    }

}
