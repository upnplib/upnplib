// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-02-17

/*!
 * \file
 *
 * \brief SSDPResultDataCallback.
 *
 * \author Marcelo Roberto Jimenez
 */

#include "config.h"

#include "SSDPResultData.hpp"
#include "SSDPResultDataCallback.hpp"

void SSDPResultData_Callback(const SSDPResultData* p) {
    Upnp_FunPtr callback = SSDPResultData_get_CtrlptCallback(p);
    callback(UPNP_DISCOVERY_SEARCH_RESULT, SSDPResultData_get_Param(p),
             SSDPResultData_get_Cookie(p));
}
