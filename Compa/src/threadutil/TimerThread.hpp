#ifndef COMPA_TIMERTHREAD_HPP
#define COMPA_TIMERTHREAD_HPP
/*******************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-03-06
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * * Neither name of Intel Corporation nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/
/*!
 * \file
 * \ingroup threadutil
 * \brief Manage threads that start at a given time (for internal use only).
 *
 * Because this is for internal use, parameters are NOT checked for validity.
 * The caller must ensure valid parameters.
 */

#include "ThreadPool.hpp"

/*! \brief Timeout Types. */
enum TimeoutType {
    ABS_SEC, ///< seconds from Jan 1, 1970.
    REL_SEC  ///< seconds from current time.
};

/*!
 * \brief A timer thread that allows the scheduling of a job to run at a
 * specified time in the future.
 *
 * Because the timer thread uses the thread pool there is no gurantee of
 * timing, only approximate timing.
 *
 * Uses ThreadPool, Mutex, Condition, Thread.
 */
struct TimerThread {
    ithread_mutex_t mutex;    ///< [in]
    ithread_cond_t condition; ///< [in]
    int lastEventId;          ///< [in]
    LinkedList eventQ;        ///< [in]
    int shutdown;             ///< [in]
    FreeList freeEvents;      ///< [in]
    ThreadPool* tp;           ///< [in]
};

/*!
 * \brief Initializes and starts timer thread.
 *
 * \returns
 *  On success: **0**\n
 *  On error: nonzero, returns error from ThreadPoolAddPersistent().
 */
int TimerThreadInit(
    /*! [in] Valid timer thread pointer. */
    TimerThread* timer,
    /*! [in] Valid thread pool to use. Must be started. Must be valid for
     * lifetime of timer. Timer must be shutdown BEFORE thread pool. */
    ThreadPool* tp);

/*!
 * \brief Schedules an event to run at a specified time.
 *
 * \returns
 *  On success: **0**\n
 *  On error: nonzero
 *  - EOUTOFMEM if not enough memory to schedule job.
 */
int TimerThreadSchedule(
    /*! [in] Valid timer thread pointer. */
    TimerThread* timer,
    /*! [in] time of event. Either in absolute seconds, or relative seconds in
     * the future. */
    time_t timeout,
    /*! [in] either ABS_SEC, or REL_SEC. If REL_SEC, then the event will be
     * scheduled at the current time + REL_SEC. */
    TimeoutType type,
    /*! [in] Valid Thread pool job with following fields. */
    ThreadPoolJob* job,
    Duration duration, ///< [in]
    /*! [out] Id of timer event. (can be null). */
    int* id);

/*!
 * \brief Removes an event from the timer Q.
 *
 * Events can only be removed before they have been placed in the thread pool.
 *
 * \returns
 *  On success: **0**\n
 *  On error: INVALID_EVENT_ID
 */
// Don't export function symbol; only used library intern.
int TimerThreadRemove(
    /*! [in] Valid timer thread pointer. */
    TimerThread* timer,
    /*! [in] Id of event to remove. */
    int id,
    /*! [out] Thread pool job. */
    ThreadPoolJob* out);

/*!
 * \brief Shutdown the timer thread.
 *
 * Events scheduled in the future will NOT be run. Timer thread should be
 * shutdown BEFORE it's associated thread pool.
 *
 * \returns Always **0**
 */
int TimerThreadShutdown(
    /*! [in] Valid timer thread pointer. */
    TimerThread* timer);

#endif /* COMPA_TIMERTHREAD_HPP */
