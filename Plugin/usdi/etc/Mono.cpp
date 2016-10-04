#include "pch.h"
#include "Mono.h"
#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif


// mono functions
void *g_mono_dll;

MonoDomain*  (*mono_domain_get)(void);
MonoAssembly*(*mono_domain_assembly_open)(MonoDomain *domain, const char *assemblyName);
MonoImage*   (*mono_assembly_get_image)(MonoAssembly *assembly);

MonoThread*  (*mono_thread_current)(void);
MonoThread*  (*mono_thread_attach)(MonoDomain *domain);
void         (*mono_thread_detach)(MonoThread *thread);
void         (*mono_thread_suspend_all_other_threads)();
void         (*mono_thread_abort_all_other_threads)();
void         (*mono_jit_thread_attach)(MonoDomain *domain);

void         (*mono_add_internal_call)(const char *name, void *method);
MonoObject*  (*mono_runtime_invoke)(MonoMethod *method, MonoObject *obj, void **params, void **exc);

MonoClass*   (*mono_class_from_name)(MonoImage *image, const char *namespaceString, const char *classnameString);
MonoMethod*  (*mono_class_get_method_from_name)(MonoClass *klass, const char *name, int param_count);

MonoClass*   (*mono_object_get_class)(MonoObject *obj);
gpointer     (*mono_object_unbox)(MonoObject *obj);

guint32      (*mono_gchandle_new)(MonoObject *obj, gboolean pinned);
void         (*mono_gchandle_free)(guint32 gchandle);


static void initialize_mono_functions()
{
#ifdef _WIN32
    auto mono = ::GetModuleHandleA("mono.dll");
    g_mono_dll = mono;
    if (mono) {
#define Import(Name) (void*&)Name = ::GetProcAddress(mono, #Name)

        Import(mono_domain_get);
        Import(mono_domain_assembly_open);
        Import(mono_assembly_get_image);

        Import(mono_thread_current);
        Import(mono_thread_attach);
        Import(mono_thread_detach);
        Import(mono_thread_suspend_all_other_threads);
        Import(mono_thread_abort_all_other_threads);
        Import(mono_jit_thread_attach);

        Import(mono_add_internal_call);
        Import(mono_runtime_invoke);

        Import(mono_class_from_name);
        Import(mono_class_get_method_from_name);

        Import(mono_object_get_class);
        Import(mono_object_unbox);

        Import(mono_gchandle_new);
        Import(mono_gchandle_free);

#undef Import
    }
#else
    // todo
#endif
}

static struct _initialize_mono_functions {
    _initialize_mono_functions() { initialize_mono_functions(); }
} g_initialize_mono_functions;

