// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-02-05

#include "umock/timeh.inc"

namespace umock {

time_t TimehReal::time(time_t* tloc) { return ::time(tloc); }

// This constructor is used to inject the pointer to the real function.
Timeh::Timeh(TimehReal* a_ptr_realObj) {
    m_ptr_workerObj = (TimehInterface*)a_ptr_realObj;
}

// This constructor is used to inject the pointer to the mocking function.
Timeh::Timeh(TimehInterface* a_ptr_mockObj) {
    m_ptr_oldObj = m_ptr_workerObj;
    m_ptr_workerObj = a_ptr_mockObj;
}

// The destructor is ussed to restore the old pointer.
Timeh::~Timeh() { m_ptr_workerObj = m_ptr_oldObj; }

// Methods
time_t Timeh::time(time_t* tloc) { return m_ptr_workerObj->time(tloc); }

// On program start create an object and inject pointer to the real functions.
// This will exist until program end.
TimehReal timeh_realObj;
UPNPLIB_API Timeh time_h(&timeh_realObj);

} // namespace umock
