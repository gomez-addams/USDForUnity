#include "pch.h"

#ifdef _WIN32
#include "usdiInternal.h"
#include "etc/Hook.h"
#include "etc/Mono.h"
#include "etc/MonoWrapper.h"

BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD fdwReason, LPVOID /*lpvReserved*/)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
//#ifndef usdiEnableMonoBindingThreadGuard
        // redirect _mono_thread_suspend_all_other_threads to _mono_thread_abort_all_other_threads to avoid
        // Mono wait TBB worker threads forever...
#ifdef usdiEnableMonoBinding
        if (g_mono_dll) {
            ForceWrite(_mono_thread_suspend_all_other_threads, 14, [=]() {
                EmitJumpInstruction(_mono_thread_suspend_all_other_threads, _mono_thread_abort_all_other_threads);
            });
        }
#endif // usdiEnableMonoBinding
//#endif
    }
    return TRUE;
}
#endif // _WIN32
