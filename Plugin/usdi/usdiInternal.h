#pragma once



#define usdiImpl

#define usdiLogError(...)       usdi::LogImpl(1, "usdi error: " __VA_ARGS__)
#define usdiLogWarning(...)     usdi::LogImpl(2, "usdi warning: " __VA_ARGS__)
#define usdiLogInfo(...)        usdi::LogImpl(3, "usdi info: " __VA_ARGS__)
#ifdef usdiDebug
    #define usdiLogTrace(...)   usdi::LogImpl(4, "usdi trace: " __VA_ARGS__)
    #define usdiLogDetail(...)  usdi::LogImpl(5, "usdi trace: " __VA_ARGS__)
    #define usdiTraceFunc(...)  usdi::TraceFuncImpl _trace_(__FUNCTION__)
#else
    #define usdiLogTrace(...)
    #define usdiLogDetail(...)
    #define usdiTraceFunc(...)
#endif

#ifdef usdiDbgVTune
    #define usdiVTuneScope(Name)    static usdi::VTuneTask s_vtune_task_##__LINE__(Name); usdi::VTuneScope _vtune_scope_##__LINE__(s_vtune_task_##__LINE__);
#else
    #define usdiVTuneScope(...)
#endif


#include "usdi.h"

#define usdiInvalidTime DBL_MIN

namespace usdi {

void LogImpl(int level, const char *format, ...);
struct TraceFuncImpl
{
    const char *m_func;
    TraceFuncImpl(const char *func);
    ~TraceFuncImpl();
};

#ifdef usdiDbgVTune
class VTuneTask
{
public:
    VTuneTask(const char *label);
    ~VTuneTask();
    void begin();
    void end();
private:
    __itt_string_handle *m_name = nullptr;
};

class VTuneScope
{
public:
    VTuneScope(VTuneTask& parent) : m_parent(parent) { m_parent.begin(); }
    ~VTuneScope() { m_parent.end(); }
private:
    VTuneTask& m_parent;
};
#endif

extern const float Rad2Deg;
extern const float Deg2Rad;

float2 operator*(const float2& l, float r);
float3 operator+(const float3& l, const float3& r);
float3 operator-(const float3& l, const float3& r);
float3 operator*(const float3& l, float r);
float4 operator*(const float4& l, float r);
quatf operator*(const quatf& l, float r);
quatf operator*(const quatf& l, const quatf& r);
float2& operator*=(float2& l, float r);
float3& operator*=(float3& l, float r);
float4& operator*=(float4& l, float r);
quatf& operator*=(quatf& l, float r);

} // namespace usdi

#include "usdiInternal.i"
