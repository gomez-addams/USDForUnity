#include "pch.h"
#include "usdiInternal.h"
#include "usdiAttribute.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiMesh.h"
#include "usdiUtils.h"

namespace usdi {

const int usdiMaxVertices = 64998;

static inline void CountIndices(const VtArray<int> &counts, int& num_indices, int& num_indices_triangulated)
{
    int reti = 0, rett = 0;
    size_t num_faces = counts.size();
    for (size_t fi = 0; fi < num_faces; ++fi)
    {
        auto f = counts[fi];
        reti += f;
        rett += (f - 2) * 3;
    }
    num_indices = reti;
    num_indices_triangulated = rett;
}

template<class CountArray, class IndexArray>
static inline void TriangulateIndices(int *triangulated, const CountArray &counts, const IndexArray *indices, bool swap_face)
{
    const int i1 = swap_face ? 2 : 1;
    const int i2 = swap_face ? 1 : 2;
    size_t num_faces = counts.size();

    int n = 0;
    int i = 0;
    if (indices) {
        for (size_t fi = 0; fi < num_faces; ++fi) {
            int ngon = counts[fi];
            for (int ni = 0; ni < ngon - 2; ++ni) {
                triangulated[i + 0] = (*indices)[n + 0];
                triangulated[i + 1] = (*indices)[n + ni + i1];
                triangulated[i + 2] = (*indices)[n + ni + i2];
                i += 3;
            }
            n += ngon;
        }
    }
    else {
        for (size_t fi = 0; fi < num_faces; ++fi) {
            int ngon = counts[fi];
            for (int ni = 0; ni < ngon - 2; ++ni) {
                triangulated[i + 0] = n + 0;
                triangulated[i + 1] = n + ni + i1;
                triangulated[i + 2] = n + ni + i2;
                i += 3;
            }
            n += ngon;
        }
    }
}


void SubmeshSample::clear()
{
    points.clear();
    normals.clear();
    uvs.clear();
    indices.clear();
    bounds_min = {}, bounds_max = {};
    center = {}, extents = {};
}

void MeshSample::clear()
{
    points.clear();
    velocities.clear();
    normals.clear();
    uvs.clear();
    counts.clear();
    indices.clear();
    indices_triangulated;
    bounds_min = {}, bounds_max = {};
    center = {}, extents = {};
}


#define usdiUVAttrName "primvars:uv"
#define usdiUVAttrName2 "uv"

const char *Mesh::UsdTypeName = "Mesh";

Mesh::Mesh(Context *ctx, Schema *parent, const UsdPrim& prim)
    : super(ctx, parent, prim)
    , m_mesh(prim)
{
    usdiLogTrace("Mesh::Mesh(): %s\n", getPath());
    if (!m_mesh) { usdiLogError("Mesh::Mesh(): m_mesh is invalid\n"); }

    m_attr_uv = findAttribute(usdiUVAttrName);
    if (!m_attr_uv) {
        m_attr_uv = findAttribute(usdiUVAttrName2);
    }
    if (!m_attr_uv) {
        m_attr_uv = createAttribute(usdiUVAttrName, AttributeType::Float2Array);
    }
}

Mesh::Mesh(Context *ctx, Schema *parent, const char *name, const char *type)
    : super(ctx, parent, name, type)
    , m_mesh(m_prim)
{
    usdiLogTrace("Mesh::Mesh(): %s\n", getPath());
    if (!m_mesh) { usdiLogError("Mesh::Mesh(): m_mesh is invalid\n"); }

    m_attr_uv = createAttribute(usdiUVAttrName, AttributeType::Float2Array);
}

Mesh::~Mesh()
{
    usdiLogTrace("Mesh::~Mesh(): %s\n", getPath());
}

const MeshSummary& Mesh::getSummary() const
{
    if (m_summary_needs_update) {
        getTimeRange(m_summary.start, m_summary.end);
        m_summary.has_uvs = m_attr_uv && m_attr_uv->hasValue();
        m_summary.has_normals = m_mesh.GetNormalsAttr().HasValue();
        m_summary.has_velocities = m_mesh.GetVelocitiesAttr().HasValue();

        if (m_mesh.GetPointsAttr().ValueMightBeTimeVarying()) {
            m_summary.topology_variance = TopologyVariance::Homogenous;
        }
        if (m_mesh.GetFaceVertexCountsAttr().ValueMightBeTimeVarying() ||
            m_mesh.GetFaceVertexIndicesAttr().ValueMightBeTimeVarying()) {
            m_summary.topology_variance = TopologyVariance::Heterogenous;
        }

        m_summary_needs_update = false;
    }
    return m_summary;
}


void Mesh::updateSample(Time t_)
{
    super::updateSample(t_);
    if (m_update_flag.bits == 0) { return; }
    if (m_update_flag.variant_set_changed) { m_summary_needs_update = true; }


    auto t = UsdTimeCode(t_);
    const auto& conf = getImportConfig();

    // swap front sample
    if (!m_front_sample) {
        m_front_sample = &m_sample[0];
        m_front_submesh = &m_submeshes[0];
    }
    else if(conf.double_buffering) {
        if (m_front_sample == &m_sample[0]) {
            m_front_sample = &m_sample[1];
            m_front_submesh = &m_submeshes[1];
        }
        else {
            m_front_sample = &m_sample[0];
            m_front_submesh = &m_submeshes[0];
        }
    }
    auto& sample = *m_front_sample;
    auto& splits = *m_front_submesh;

    m_mesh.GetPointsAttr().Get(&sample.points, t);
    m_mesh.GetVelocitiesAttr().Get(&sample.velocities, t);
    m_mesh.GetFaceVertexCountsAttr().Get(&sample.counts, t);
    m_mesh.GetFaceVertexIndicesAttr().Get(&sample.indices, t);
    if (m_attr_uv) {
        m_attr_uv->getImmediate(&sample.uvs, t_);
    }

    // apply swap_handedness and scale
    if (conf.swap_handedness) {
        InvertX((float3*)sample.points.data(), sample.points.size());
        InvertX((float3*)sample.velocities.data(), sample.velocities.size());
    }
    if (conf.scale != 1.0f) {
        Scale((float3*)sample.points.data(), conf.scale, sample.points.size());
        Scale((float3*)sample.velocities.data(), conf.scale, sample.velocities.size());
    }

    // normals
    bool needs_calculate_normals = conf.normal_calculation == NormalCalculationType::Always;
    if (!needs_calculate_normals) {
        if (m_mesh.GetNormalsAttr().Get(&sample.normals, t)) {
            if (conf.swap_handedness) {
                InvertX((float3*)sample.normals.data(), sample.normals.size());
            }
        }
        else {
            if (conf.normal_calculation == NormalCalculationType::WhenMissing) {
                needs_calculate_normals = true;
            }
            else {
                // no normal data is present and no recalculation is required.
                // just allocate empty normal array.
                sample.normals.resize(sample.points.size());
                memset(sample.normals.data(), 0, sizeof(float3)*sample.normals.size());
            }
        }
    }

    // indices
    bool needs_calculate_indices =
        m_num_indices_triangulated == 0 ||
        getSummary().topology_variance == TopologyVariance::Heterogenous ||
        m_update_flag.variant_set_changed;
    bool needs_copy_indices =
        sample.indices_triangulated.size() != m_sample[0].indices_triangulated.size() ||
        m_update_flag_prev.variant_set_changed;
    if (needs_calculate_indices) {
        CountIndices(sample.counts, m_num_indices, m_num_indices_triangulated);
        if (conf.triangulate || needs_calculate_normals) {
            sample.indices_triangulated.resize(m_num_indices_triangulated);
            TriangulateIndices(sample.indices_triangulated.data(), sample.counts, &sample.indices, conf.swap_faces);
        }
    }
    else if (needs_copy_indices) {
        sample.indices_triangulated = m_sample[0].indices_triangulated;
    }

    // calculate normals if needed
    if (needs_calculate_normals) {
        sample.normals.resize(sample.points.size());
        CalculateNormals((float3*)sample.normals.data(), (const float3*)sample.points.cdata(), sample.indices_triangulated.cdata(),
            sample.points.size(), sample.indices_triangulated.size());
    }

    // bounds
    ComputeBounds((const float3*)sample.points.cdata(), sample.points.size(), sample.bounds_min, sample.bounds_max);
    sample.center = (sample.bounds_min + sample.bounds_max) * 0.5f;
    sample.extents = (sample.bounds_max - sample.bounds_min) * 0.5f;


    // mesh split

    bool points_are_expanded = sample.points.size() == m_num_indices;
    bool normals_are_expanded = sample.normals.size() == m_num_indices;
    bool uvs_are_expanded = sample.uvs.size() == m_num_indices;

    bool needs_split = false;
    if (conf.split_mesh) {
        needs_split = sample.points.size() > usdiMaxVertices || points_are_expanded || normals_are_expanded || uvs_are_expanded;
    }
    if (!needs_split) { return; }

    int num_splits = ceildiv(m_num_indices_triangulated, usdiMaxVertices);
    splits.resize(num_splits);

    for (int nth = 0; nth < num_splits; ++nth) {
        auto& sms = splits[nth];
        int ibegin = usdiMaxVertices * nth;
        int iend = std::min<int>(usdiMaxVertices * (nth+1), m_num_indices_triangulated);
        int isize = iend - ibegin;

        {
            sms.indices.resize(isize);
            for (int i = 0; i < isize; ++i) { sms.indices[i] = i; }
        }
        CopyWithIndices(sms.points, sample.points, sample.indices_triangulated, ibegin, iend, !points_are_expanded);
        CopyWithIndices(sms.normals, sample.normals, sample.indices_triangulated, ibegin, iend, !normals_are_expanded);
        CopyWithIndices(sms.uvs, sample.uvs, sample.indices_triangulated, ibegin, iend, !uvs_are_expanded);

        ComputeBounds((float3*)sms.points.cdata(), sms.points.size(), sms.bounds_min, sms.bounds_max);
        sms.center = (sms.bounds_min + sms.bounds_max) * 0.5f;
        sms.extents = sms.bounds_max - sms.bounds_min;
    }
}

bool Mesh::readSample(MeshData& dst, Time t, bool copy)
{
    if (t != m_time_prev) { updateSample(t); }

    if (!m_front_sample) { return false; }

    const auto& sample = *m_front_sample;
    const auto& splits = *m_front_submesh;

    dst.num_points = (uint)sample.points.size();
    dst.num_counts = (uint)sample.counts.size();
    dst.num_indices = (uint)sample.indices.size();
    dst.num_indices_triangulated = m_num_indices_triangulated;
    dst.num_submeshes = (uint)splits.size();
    dst.center = sample.center;
    dst.extents = sample.extents;

    if (copy) {
        if (dst.points && !sample.points.empty()) {
            memcpy(dst.points, sample.points.cdata(), sizeof(float3) * dst.num_points);
        }
        if (dst.velocities && !sample.velocities.empty()) {
            memcpy(dst.velocities, sample.velocities.cdata(), sizeof(float3) * dst.num_points);
        }
        if (dst.normals && !sample.normals.empty()) {
            memcpy(dst.normals, sample.normals.cdata(), sizeof(float3) * dst.num_points);
        }
        if (dst.uvs && !sample.uvs.empty()) {
            memcpy(dst.uvs, sample.uvs.cdata(), sizeof(float2) * dst.num_points);
        }
        if (dst.counts && !sample.counts.empty()) {
            memcpy(dst.counts, sample.counts.cdata(), sizeof(int) * dst.num_counts);
        }
        if (dst.indices && !sample.indices.empty()) {
            memcpy(dst.indices, sample.indices.cdata(), sizeof(int) * dst.num_indices);
        }
        if (dst.indices_triangulated && !sample.indices_triangulated.empty()) {
            memcpy(dst.indices_triangulated, sample.indices_triangulated.cdata(), sizeof(int) * m_num_indices_triangulated);
        }

        if (dst.submeshes) {
            for (size_t i = 0; i < dst.num_submeshes; ++i) {
                const auto& ssrc = splits[i];
                auto& sdst = dst.submeshes[i];
                sdst.num_points = (uint)ssrc.points.size();
                sdst.center = ssrc.center;
                sdst.extents = ssrc.extents;

                if (sdst.indices && !ssrc.indices.empty()) {
                    memcpy(sdst.indices, ssrc.indices.cdata(), sizeof(int) * sdst.num_points);
                }
                if (sdst.points && !ssrc.points.empty()) {
                    memcpy(sdst.points, ssrc.points.cdata(), sizeof(float3) * sdst.num_points);
                }
                if (sdst.normals && !ssrc.normals.empty()) {
                    memcpy(sdst.normals, ssrc.normals.cdata(), sizeof(float3) * sdst.num_points);
                }
                if (sdst.uvs && !ssrc.uvs.empty()) {
                    memcpy(sdst.uvs, ssrc.uvs.cdata(), sizeof(float2) * sdst.num_points);
                }
            }
        }
    }
    else {
        dst.points = (float3*)sample.points.cdata();
        dst.velocities = (float3*)sample.velocities.cdata();
        dst.normals = (float3*)sample.normals.cdata();
        dst.uvs = (float2*)sample.uvs.cdata();
        dst.counts = (int*)sample.counts.cdata();
        dst.indices = (int*)sample.indices.cdata();
        dst.indices_triangulated = (int*)sample.indices_triangulated.cdata();

        if (dst.submeshes) {
            for (size_t i = 0; i < dst.num_submeshes; ++i) {
                const auto& ssrc = splits[i];
                auto& sdst = dst.submeshes[i];
                sdst.num_points = (uint)ssrc.points.size();
                if (sdst.indices && !ssrc.indices.empty()) {
                    sdst.indices = (int*)ssrc.indices.cdata();
                }
                if (sdst.points && !ssrc.points.empty()) {
                    sdst.points = (float3*)ssrc.points.cdata();
                }
                if (sdst.normals && !ssrc.normals.empty()) {
                    sdst.normals = (float3*)ssrc.normals.cdata();
                }
                if (sdst.uvs && !ssrc.uvs.empty()) {
                    sdst.uvs = (float2*)ssrc.uvs.cdata();
                }
            }
        }
    }

    return dst.num_points > 0;
}

bool Mesh::writeSample(const MeshData& src, Time t_)
{
    auto t = UsdTimeCode(t_);
    const auto& conf = getExportConfig();

    MeshSample& sample = m_sample[0];

    if (src.points) {
        sample.points.assign((GfVec3f*)src.points, (GfVec3f*)src.points + src.num_points);
        if (conf.swap_handedness) {
            InvertX((float3*)sample.points.data(), sample.points.size());
        }
        if (conf.scale != 1.0f) {
            Scale((float3*)sample.points.data(), conf.scale, sample.points.size());
        }
    }

    if (src.velocities) {
        sample.points.assign((GfVec3f*)src.velocities, (GfVec3f*)src.velocities + src.num_points);
        if (conf.swap_handedness) {
            InvertX((float3*)sample.velocities.data(), sample.velocities.size());
        }
        if (conf.scale != 1.0f) {
            Scale((float3*)sample.velocities.data(), conf.scale, sample.velocities.size());
        }
    }

    if (src.normals) {
        sample.normals.assign((GfVec3f*)src.normals, (GfVec3f*)src.normals + src.num_points);
        if (conf.swap_handedness) {
            InvertX((float3*)sample.normals.data(), sample.normals.size());
        }
    }

    if (src.uvs) {
        sample.uvs.assign((GfVec2f*)src.uvs, (GfVec2f*)src.uvs + src.num_points);
    }

    if (src.counts) {
        sample.counts.assign(src.counts, src.counts + src.num_counts);
    }
    else if (src.indices) {
        // assume all faces are triangles
        size_t ntriangles = src.num_indices / 3;
        sample.counts.assign(ntriangles, 3);
    }

    if (src.indices) {
        if (conf.swap_faces) {
            auto copy_with_swap = [](VtArray<int>& dst_indices, const int *src_indices, const VtArray<int>& counts) {
                int i = 0;
                for (int ngon : counts) {
                    for (int ni = 0; ni < ngon; ++ni) {
                        int ini = ngon - ni - 1;
                        dst_indices[i + ni] = src_indices[i + ini];
                    }
                    i += ngon;
                }
            };

            sample.indices.resize(src.num_indices);
            copy_with_swap(sample.indices, src.indices, sample.counts);
        }
        else {
            sample.indices.assign(src.indices, src.indices + src.num_indices);
        }
    }


    bool  ret = m_mesh.GetPointsAttr().Set(sample.points, t);
    if (src.velocities) {
        m_mesh.GetVelocitiesAttr().Set(sample.velocities, t);
    }
    if (src.normals) {
        m_mesh.GetNormalsAttr().Set(sample.normals, t);
    }
    if (src.indices) {
        m_mesh.GetFaceVertexCountsAttr().Set(sample.counts, t);
        m_mesh.GetFaceVertexIndicesAttr().Set(sample.indices, t);
    }
    if (src.uvs && m_attr_uv) {
        m_attr_uv->setImmediate(&sample.uvs, t_);
    }

    m_summary_needs_update = true;
    return ret;
}

} // namespace usdi
