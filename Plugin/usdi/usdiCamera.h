#pragma once

namespace usdi {

class Camera : public Xform
{
typedef Xform super;
friend class Context;
protected:
    Camera(Context *ctx, Schema *parent, const UsdGeomCamera& xf);
    Camera(Context *ctx, Schema *parent, const char *name, const char *type = "Camera");
    ~Camera() override;

public:
    using UsdType = UsdGeomCamera;

    UsdGeomCamera&      getUSDSchema() override;
    void                updateSample(Time t) override;

    const CameraSummary& getSummary() const;
    bool                readSample(CameraData& dst, Time t);
    bool                writeSample(const CameraData& src, Time t);

private:
    UsdGeomCamera       m_cam;
    CameraData          m_sample;
    mutable CameraSummary m_summary;
};

} // namespace usdi
