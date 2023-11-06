#ifndef PUPNP_THREADPOOL_INIT_HPP
#define PUPNP_THREADPOOL_INIT_HPP
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-11-06

#include <UpnpGlobal.hpp> // for EXPORT_SPEC
#include <ThreadPool.hpp>

namespace pupnp {

// Class to initialize and shutdown ThreadPools
// --------------------------------------------
// This is used on tests that need ThreadPools. With the constructor you can
// specify relevant test parameter, which are the shutdown flag and the maximal
// value of possible jobs.
// a_shutdown: if this flag is set then the threadpool management does not add
//             new jobs to a threadpool. This can be used when the isolated
//             test needs threadpools to be initialized but will have timing
//             problems when running in a thread.
// a_maxjobstotal: for tests this is mostly set to 1 so there is only one job in
//                one thread running. With setting it to 0 it can be tested if
//                the Unit under test will run in a thread. You will get an
//                error message "to much jobs: 0".
class EXPORT_SPEC CThreadPoolInit {
  public:
    CThreadPoolInit(::ThreadPool& a_threadpool, const bool a_shutdown = 0,
                    const int a_maxJobsTotal = DEFAULT_MAX_JOBS_TOTAL);

    virtual ~CThreadPoolInit();

  private:
    ThreadPool& m_threadpool;
};

} // namespace pupnp

#endif // PUPNP_THREADPOOL_INIT_HPP
