#pragma once

namespace usdi {


struct SplitedMeshSample
{
    VtArray<GfVec3f> points;
    VtArray<GfVec3f> normals;
    VtArray<GfVec2f> uvs;
    VtArray<int>     indices;
};

struct MeshSample
{
    VtArray<GfVec3f> points;
    VtArray<GfVec3f> velocities;
    VtArray<GfVec3f> normals;
    VtArray<GfVec2f> uvs;
    VtArray<int>     counts;
    VtArray<int>     indices;
    VtArray<int>     indices_triangulated;
};


class Mesh : public Xform
{
typedef Xform super;
public:
    Mesh(Context *ctx, Schema *parent, const UsdGeomMesh& mesh);
    Mesh(Context *ctx, Schema *parent, const char *name);
    ~Mesh() override;

    UsdGeomMesh&        getUSDSchema() override;
    void                updateSample(Time t) override;

    const MeshSummary&  getSummary() const;
    bool                readSample(MeshData& dst, Time t, bool copy);
    bool                writeSample(const MeshData& src, Time t);

private:
    void updateSummary() const;

private:
    typedef std::vector<SplitedMeshSample> SplitedMeshSamples;

    UsdGeomMesh         m_mesh;
    MeshSample          m_sample[2], *m_front_sample = nullptr;
    SplitedMeshSamples  m_splits[2], *m_front_splits = nullptr;
    Attribute           *m_attr_uv = nullptr;

    mutable bool        m_summary_needs_update = true;
    mutable MeshSummary m_summary;
    mutable int         m_num_indices = 0;
    mutable int         m_num_indices_triangulated = 0;
};

} // namespace usdi
