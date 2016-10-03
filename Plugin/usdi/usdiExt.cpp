﻿#include "pch.h"

#ifdef usdiEnableUnityExtension
#include "usdiInternal.h"
#include "usdiAttribute.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiCamera.h"
#include "usdiMesh.h"
#include "usdiPoints.h"
#include "usdiContext.h"

#include "etc/Mono.h"
#include "usdiExt.h"
#include "ext/usdiTask.h"
#include "ext/usdiUnity.h"


namespace usdi {

    VertexCommandManager g_vtx_task_manager;

} // namespace usdi


extern "C" {

usdiAPI usdi::Handle usdiVtxCmdCreate(const char *dbg_name)
{
    usdiTraceFunc();
    return usdi::g_vtx_task_manager.createCommand(dbg_name);
}

usdiAPI void usdiVtxCmdDestroy(usdi::Handle h)
{
    usdiTraceFunc();
    usdi::g_vtx_task_manager.destroyCommand(h);
}

usdiAPI void usdiVtxCmdUpdate(usdi::Handle h, const usdi::MeshData *src, void *vb, void *ib)
{
    usdiTraceFunc();
    usdi::g_vtx_task_manager.update(h, src, vb, ib);
}

usdiAPI void usdiVtxCmdProcess()
{
    usdiTraceFunc();
    usdiVTuneScope("usdiVtxCmdKick");
    usdi::g_vtx_task_manager.process();
}

usdiAPI void usdiVtxCmdWait()
{
    usdiTraceFunc();
    usdiVTuneScope("usdiVtxCmdKick");
    usdi::g_vtx_task_manager.wait();
}




usdiAPI void usdiTaskDestroy(usdi::Task *t)
{
    usdiTraceFunc();
    delete t;
}

usdiAPI void usdiTaskRun(usdi::Task *t)
{
    usdiTraceFunc();
    t->run();
}

usdiAPI bool usdiTaskIsRunning(usdi::Task *t)
{
    usdiTraceFunc();
    return t->isRunning();
}

usdiAPI void usdiTaskWait(usdi::Task *t)
{
    usdiTraceFunc();
    t->wait();
}

usdiAPI usdi::Task* usdiTaskCreateMonoDelegate(usdi::MonoDelegate func, void *arg, const char *name)
{
    usdiTraceFunc();
    return new usdi::Task([=]() {
        usdi::MonoThreadScope mts;
        func(arg);
    }, name);
}

usdiAPI usdi::Task* usdiTaskCreateMeshReadSample(usdi::Mesh *mesh, usdi::MeshData *dst, const usdi::Time *t)
{
    usdiTraceFunc();
    if (!mesh || !dst || !t) { return 0; }
    return new usdi::Task([=]() {
        mesh->readSample(*dst, *t, true);
    });
}

usdiAPI usdi::Task* usdiTaskCreatePointsReadSample(usdi::Points *points, usdi::PointsData *dst, const usdi::Time *t)
{
    usdiTraceFunc();
    if (!points || !dst || !t) { return 0; }
    return new usdi::Task([=]() {
        points->readSample(*dst, *t, true);
    });
}

usdiAPI usdi::Task* usdiTaskCreateAttrReadSample(usdi::Attribute *attr, usdi::AttributeData *dst, const usdi::Time *t)
{
    usdiTraceFunc();
    if (!attr || !dst || !t) { return 0; }
    return new usdi::Task([=]() {
        attr->readSample(*dst, *t, true);
    });
}

usdiAPI usdi::Task* usdiTaskCreateComposite(usdi::Task **tasks, int num)
{
    usdiTraceFunc();
    if (!tasks || num == 0) { return 0; }
    return new usdi::Task([=]() {
        for (int i = 0; i < num; ++i) {
            tasks[i]->run(false);
        }
    });
}



usdiAPI void usdiUniForceAssignTRS(void *monoobj, const usdi::XformData *data)
{
    usdiTraceFunc();
    usdi::ForceAssignTRS(monoobj, *data);
}

usdiAPI void usdiUniForceAssignBounds(void *monoobj, const usdi::float3 *center, const usdi::float3 *extents)
{
    usdiTraceFunc();
    usdi::ForceAssignBounds(monoobj, *center, *extents);
}

} // extern "C"
#endif // usdiEnableUnityExtension
