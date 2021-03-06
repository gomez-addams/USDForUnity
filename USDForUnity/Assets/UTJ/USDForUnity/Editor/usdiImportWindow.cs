using System;
using System.IO;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    public class usdiImportWindow : EditorWindow
    {
        public string m_path;
        usdi.ImportSettings m_importOptions = usdi.ImportSettings.default_value;
        double m_initialTime = 0.0;
        bool m_forceSingleThread = false;
        bool m_directVBUpdate = true;

        public static void Open(string path)
        {
            usdiImportWindow window = (usdiImportWindow)EditorWindow.GetWindow(typeof(usdiImportWindow));
            window.titleContent = new GUIContent("Import Settings");
            window.m_path = path;
            window.Show();
        }

        public static usdiStream InstanciateUSD(string path, Action<usdiStream> modifier)
        {
            var go = new GameObject();
            go.name = Path.GetFileNameWithoutExtension(path);

            var usd = go.AddComponent<usdiStream>();
            modifier.Invoke(usd);
            usd.Load(path);
            return usd;
        }

        void OnGUI()
        {
            m_importOptions.interpolation = (usdi.InterpolationType)EditorGUILayout.EnumPopup("Interpolation", (Enum)m_importOptions.interpolation);
            m_importOptions.normalCalculation = (usdi.NormalCalculationType)EditorGUILayout.EnumPopup("Normal Calculation", (Enum)m_importOptions.normalCalculation);
            m_importOptions.tangentCalculation = (usdi.TangentCalculationType)EditorGUILayout.EnumPopup("Tangent Calculation", (Enum)m_importOptions.tangentCalculation);
            m_importOptions.scale = EditorGUILayout.FloatField("Scale", m_importOptions.scale);
            m_importOptions.swapHandedness = EditorGUILayout.Toggle("Swap Handedness", m_importOptions.swapHandedness);
            m_importOptions.swapFaces = EditorGUILayout.Toggle("Swap Faces", m_importOptions.swapFaces);
            EditorGUILayout.Space();
            m_initialTime = EditorGUILayout.FloatField("Initial Time", (float)m_initialTime);
            m_forceSingleThread = EditorGUILayout.Toggle("Force Single Thread", m_forceSingleThread);
            m_directVBUpdate = EditorGUILayout.Toggle("Direct VB Update", m_directVBUpdate);

            GUILayout.Space(10.0f);

            if (GUILayout.Button("Import"))
            {
                var usd = InstanciateUSD(m_path, (stream) => {
                    stream.importSettings = m_importOptions;
                    stream.playTime = m_initialTime;
                    stream.forceSingleThread = m_forceSingleThread;
                    stream.directVBUpdate = m_directVBUpdate;
                });
                Selection.activeGameObject = usd.gameObject;
                Close();
            }
        }

    }
}
