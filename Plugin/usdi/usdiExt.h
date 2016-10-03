#pragma once

#ifdef usdiEnableUnityExtension
#include "usdi.h"

// ext

namespace usdi {
    typedef size_t Handle; // async handle
    class Task;
    typedef void(__stdcall *MonoDelegate)(void*);
} // namespace usdi

extern "C" {

usdiAPI usdi::Handle    usdiVtxCmdCreate(const char *dbg_name);
usdiAPI void            usdiVtxCmdDestroy(usdi::Handle h);
usdiAPI void            usdiVtxCmdUpdate(usdi::Handle h, const usdi::MeshData *src, void *vb, void *ib);
usdiAPI void            usdiVtxCmdProcess();
usdiAPI void            usdiVtxCmdWait();

usdiAPI void            usdiTaskDestroy(usdi::Task *t);
usdiAPI void            usdiTaskRun(usdi::Task *t);
usdiAPI bool            usdiTaskIsRunning(usdi::Task *t);
usdiAPI void            usdiTaskWait(usdi::Task *t);
usdiAPI usdi::Task*     usdiTaskCreateMonoDelegate(usdi::MonoDelegate func, void *arg, const char *dbg_name);
usdiAPI usdi::Task*     usdiTaskCreateMeshReadSample(usdi::Mesh *mesh, usdi::MeshData *dst, const usdi::Time *t);
usdiAPI usdi::Task*     usdiTaskCreatePointsReadSample(usdi::Points *points, usdi::PointsData *dst, const usdi::Time *t);
usdiAPI usdi::Task*     usdiTaskCreateAttrReadSample(usdi::Attribute *attr, usdi::AttributeData *dst, const usdi::Time *t);
usdiAPI usdi::Task*     usdiTaskCreateComposite(usdi::Task **tasks, int num);

usdiAPI void usdiUniForceAssignTRS(void *monoobj, const usdi::XformData *data);
usdiAPI void usdiUniForceAssignBounds(void *monoobj, const usdi::float3 *center, const usdi::float3 *extents);

} // extern "C"
#endif // usdiEnableUnityExtension
