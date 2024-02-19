#ifndef COMPA_SSDPLIB_HPP
#define COMPA_SSDPLIB_HPP
// Copyright (C) 2024+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-02-21

// This file is only for compatibility with old Pupnp code. It should not be
// included into code for the compa library.

#if EXCLUDE_SSDP == 0
#include <ssdp_common.hpp>

#ifdef INCLUDE_CLIENT_APIS
#include <ssdp_ctrlpt.hpp>
#endif

#ifdef INCLUDE_DEVICE_APIS
#include <ssdp_device.hpp>
#endif

#endif // EXCLUDE_SSDP

#endif // COMPA_SSDPLIB_HPP
