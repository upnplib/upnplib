// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-02-27

#ifndef UPNPLIB_SSDPRESULTDATA_HPP
#define UPNPLIB_SSDPRESULTDATA_HPP

/*!
 * \file
 *
 * \brief Header file for SSDPResultData methods.
 *
 * \author Marcelo Roberto Jimenez
 */
#include <stdlib.h> /* for size_t */

#include "UpnpGlobal.hpp" /* for EXPORT_SPEC */

#include "UpnpDiscovery.hpp"
#include "Callback.hpp"

/*!
 * SSDPResultData
 */
typedef struct s_SSDPResultData SSDPResultData;

/*! Constructor */
EXPORT_SPEC SSDPResultData* SSDPResultData_new();
/*! Destructor */
EXPORT_SPEC void SSDPResultData_delete(SSDPResultData* p);
/*! Copy Constructor */
EXPORT_SPEC SSDPResultData* SSDPResultData_dup(const SSDPResultData* p);
/*! Assignment operator */
EXPORT_SPEC int SSDPResultData_assign(SSDPResultData* p,
                                      const SSDPResultData* q);

/*! SSDPResultData_get_Param */
EXPORT_SPEC const UpnpDiscovery*
SSDPResultData_get_Param(const SSDPResultData* p);
/*! SSDPResultData_set_Param */
EXPORT_SPEC int SSDPResultData_set_Param(SSDPResultData* p,
                                         const UpnpDiscovery* n);

/*! SSDPResultData_get_Cookie */
EXPORT_SPEC void* SSDPResultData_get_Cookie(const SSDPResultData* p);
/*! SSDPResultData_set_Cookie */
EXPORT_SPEC int SSDPResultData_set_Cookie(SSDPResultData* p, void* n);

/*! SSDPResultData_get_CtrlptCallback */
EXPORT_SPEC Upnp_FunPtr
SSDPResultData_get_CtrlptCallback(const SSDPResultData* p);
/*! SSDPResultData_set_CtrlptCallback */
EXPORT_SPEC int SSDPResultData_set_CtrlptCallback(SSDPResultData* p,
                                                  Upnp_FunPtr n);

#endif /* UPNPLIB_SSDPRESULTDATA_HPP */
