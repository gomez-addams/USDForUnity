#pragma once
#include "etc/Mono.h"

namespace usdi {

class uObject;
class uTransform;
class uMesh;


// native methods
extern void(uObject::*NM_Object_SetDirty)();
extern void(uTransform::*NM_Transform_SetLocalPosition)(const __m128 &pos);
extern void(uTransform::*NM_Transform_SetLocalRotation)(const __m128 &rot);
extern void(uTransform::*NM_Transform_SetLocalScale)(const __m128 &scale);
extern void(uTransform::*NM_Transform_SendTransformChanged)(int mask);
extern void(uMesh::*NM_Mesh_SetBounds)(const AABB &);


// mono classes & methods

extern MonoClass *MC_Vector2;
extern MonoClass *MC_Vector3;
extern MonoClass *MC_Quaternion;
extern MonoClass *MC_Transform;
extern MonoClass *MC_Mesh;

extern MonoMethod *MM_Transform_set_localPosition;
extern MonoMethod *MM_Transform_set_localRotation;
extern MonoMethod *MM_Transform_set_localScale;
extern MonoMethod *MM_Mesh_set_vertices;
extern MonoMethod *MM_Mesh_set_normals;
extern MonoMethod *MM_Mesh_set_uv;
extern MonoMethod *MM_Mesh_set_bounds;
extern MonoMethod *MM_Mesh_SetIndices;
extern MonoMethod *MM_Mesh_UploadMeshData;
extern MonoMethod *MM_Mesh_GetNativeVertexBufferPtr;
extern MonoMethod *MM_Mesh_GetNativeIndexBufferPtr;



void TransformAssignXformCpp(MonoObject *transform_, MonoObject *data_);
void TransformAssignXformMono(MonoObject *transform_, MonoObject *data_);
void TransformNotfyChangeCpp(MonoObject *transform_);
void TransformNotfyChangeMono(MonoObject *transform_);
void MeshAssignBoundsCpp(MonoObject *mesh_, MonoObject *center_, MonoObject  *extents_);
void MeshAssignBoundsMono(MonoObject *mesh_, MonoObject *center_, MonoObject  *extents_);

} // namespace usdi