// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-04-17

#include <pupnp/threadpool_init.hpp>
#include <upnplib/global.hpp>
#include <upnplib/synclog.hpp>

namespace pupnp {

// Class to initialize and shutdown ThreadPools
// --------------------------------------------
CThreadPoolInit::CThreadPoolInit(ThreadPool& a_threadpool,
                                 const bool a_shutdown,
                                 const int a_maxJobsTotal)
    : m_threadpool(a_threadpool) //
{
    TRACE2(this, " Construct CThreadPoolInit()")
    // Initialize the given Threadpool. nullptr means to use default
    // attributes.
    int ret = ThreadPoolInit(&m_threadpool, nullptr);
    if (ret != 0)
        throw std::runtime_error(
            UPNPLIB_LOGEXCEPT
            "MSG1045: Initializing ThreadPool fails with return number " +
            std::to_string(ret));

    if (a_shutdown) {
        TRACE("constructor CThreadPoolInit(): set shutdown flag.")
        // Prevent to add jobs without error message if set, I test jobs
        // isolated.
        m_threadpool.shutdown = 1;
    } else {
        // or allow number of threads. Use 0 with check of error message
        // to stderr to test if threads are used.
        ret = TPAttrSetMaxJobsTotal(&m_threadpool.attr, a_maxJobsTotal);
        if (ret != 0)
            throw std::runtime_error(UPNPLIB_LOGEXCEPT
                                     "MSG1046: Setting maximal jobs number "
                                     "fails with return number " +
                                     std::to_string(ret));
    }
}

CThreadPoolInit::~CThreadPoolInit() {
    TRACE2(this, " Destruct CThreadPoolInit()")
    // Shutdown the threadpool.
    ThreadPoolShutdown(&m_threadpool);
}

} // namespace pupnp
