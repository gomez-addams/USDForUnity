#pragma once

#include "usdiConfig.h"
#include "etc/Platform.h"


#define usdiImpl

#define usdiLogError(...)       usdi::LogImpl(1, "usdi error: " __VA_ARGS__)
#define usdiLogWarning(...)     usdi::LogImpl(2, "usdi warning: " __VA_ARGS__)
#define usdiLogInfo(...)        usdi::LogImpl(3, "usdi info: " __VA_ARGS__)
#ifdef usdiDebug
    #ifdef _WIN32
           #define usdiDebugBreak() ::DebugBreak()
    #else
           #define usdiDebugBreak()
    #endif
    #define usdiLogTrace(...)   usdi::LogImpl(4, "usdi trace: " __VA_ARGS__)
    #define usdiLogDetail(...)  usdi::LogImpl(5, "usdi trace: " __VA_ARGS__)
    #define usdiTraceFunc(...)  usdi::TraceFuncImpl _trace_(__FUNCTION__)
#else
    #define usdiDebugBreak()
    #define usdiLogTrace(...)
    #define usdiLogDetail(...)
    #define usdiTraceFunc(...)
#endif

#ifdef usdiDbgVTune
    #include "ittnotify.h"
    #define usdiVTuneScope(Name)    static usdi::VTuneTask s_vtune_task_##__LINE__(Name); usdi::VTuneScope _vtune_scope_##__LINE__(s_vtune_task_##__LINE__);
#else
    #define usdiVTuneScope(...)
#endif


#include "MeshUtils/muVector.h"
namespace usdi {
    using namespace mu;
} // namespace usdi

#include "usdi.h"

#define usdiInvalidTime usdiDefaultTime()

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

inline bool operator==(const ImportSettings& a, const ImportSettings& b) { return memcmp(&a, &b, sizeof(a)) == 0; }
inline bool operator!=(const ImportSettings& a, const ImportSettings& b) { return !(a == b); }
inline bool operator==(const ExportSettings& a, const ExportSettings& b) { return memcmp(&a, &b, sizeof(a)) == 0; }
inline bool operator!=(const ExportSettings& a, const ExportSettings& b) { return !(a == b); }

} // namespace usdi
