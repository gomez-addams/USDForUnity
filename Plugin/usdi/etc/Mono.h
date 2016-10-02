#pragma once


typedef void MonoThread;
typedef void MonoDomain;

extern void *g_mono_dll;
extern MonoDomain *g_mono_domain;
extern MonoDomain* (*mono_domain_get)(void);
extern MonoThread* (*mono_thread_attach)(MonoDomain *domain);
extern void(*mono_thread_detach)(MonoThread *thread);
extern void(*mono_thread_suspend_all_other_threads)();
extern void(*mono_thread_abort_all_other_threads)();
extern void(*mono_jit_thread_attach)(MonoDomain *domain);


class MonoScope
{
public:
    MonoScope();
    ~MonoScope();

private:
    MonoThread *m_mono_thread;
};

