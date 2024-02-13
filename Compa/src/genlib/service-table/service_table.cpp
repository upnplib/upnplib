/*******************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (c) 2012 France Telecom All rights reserved.
 * Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-02-13
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * - Neither name of Intel Corporation nor the names of its contributors
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
// Last compare with pupnp original source file on 2023-09-06, ver 1.14.18
/*!
 * \file
 * \brief This file defines the functions for UPnP Services.
 *
 * It defines functions for adding and removing services to and from the service
 * table, adding and accessing subscription and other attributes pertaining to
 * the service.
 * \note This is only available if the Device Module was selected at compile
 * time.
 */

#include <service_table.hpp>

#ifndef COMPA_INTERNAL_CONFIG_HPP
#error "No or wrong config.hpp header file included."
#endif

#if defined(INCLUDE_DEVICE_APIS) || defined(DOXYGEN_RUN)

#if (EXCLUDE_GENA == 0) || defined(DOXYGEN_RUN)
namespace {

/*!
 * \brief Returns pointer to service info after getting the sub-elements of the
 * service info.
 * \ingroup Eventing
 * \note Only available with the GENA module compiled in.
 *
 * \returns Pointer to the service info node.
 */
service_info* getServiceList(
    /*! [in] XML node information. */
    IXML_Node* node,
    /*! [out] Service added is returned to the output parameter. */
    service_info** end,
    /*! [in] Provides Base URL to resolve relative URL. */
    char* URLBase) {
    IXML_Node* serviceList = NULL;
    IXML_Node* current_service = NULL;
    IXML_Node* UDN = NULL;

    IXML_Node* serviceType = NULL;
    IXML_Node* serviceId = NULL;
    IXML_Node* SCPDURL = NULL;
    IXML_Node* controlURL = NULL;
    IXML_Node* eventURL = NULL;
    DOMString tempDOMString = NULL;
    service_info* head = NULL;
    service_info* current = NULL;
    service_info* previous = NULL;
    IXML_NodeList* serviceNodeList = NULL;
    long unsigned int NumOfServices = 0lu;
    long unsigned int i = 0lu;
    int fail = 0;

    if (getSubElement("UDN", node, &UDN) &&
        getSubElement("serviceList", node, &serviceList)) {
        serviceNodeList = ixmlElement_getElementsByTagName(
            (IXML_Element*)serviceList, "service");
        if (serviceNodeList) {
            NumOfServices = ixmlNodeList_length(serviceNodeList);
            for (i = 0lu; i < NumOfServices; i++) {
                current_service = ixmlNodeList_item(serviceNodeList, i);
                fail = 0;
                if (current) {
                    current->next = (service_info*)malloc(sizeof(service_info));
                    previous = current;
                    current = current->next;
                } else {
                    head = (service_info*)malloc(sizeof(service_info));
                    current = head;
                }
                if (!current) {
                    freeServiceList(head);
                    ixmlNodeList_free(serviceNodeList);
                    return NULL;
                }
                current->next = NULL;
                current->controlURL = NULL;
                current->eventURL = NULL;
                current->serviceType = NULL;
                current->serviceId = NULL;
                current->SCPDURL = NULL;
                current->active = 1;
                current->subscriptionList = NULL;
                current->TotalSubscriptions = 0;
                if ((current->UDN = getElementValue(UDN)) == 0)
                    fail = 1;
                if (!getSubElement("serviceType", current_service,
                                   &serviceType) ||
                    ((current->serviceType = getElementValue(serviceType)) ==
                     0))
                    fail = 1;
                if (!getSubElement("serviceId", current_service, &serviceId) ||
                    ((current->serviceId = getElementValue(serviceId)) == 0))
                    fail = 1;
                if (!getSubElement("SCPDURL", current_service, &SCPDURL) ||
                    ((tempDOMString = getElementValue(SCPDURL)) == 0) ||
                    ((current->SCPDURL =
                          resolve_rel_url(URLBase, tempDOMString)) == 0))
                    fail = 1;
                ixmlFreeDOMString(tempDOMString);
                tempDOMString = NULL;
                if (!(getSubElement("controlURL", current_service,
                                    &controlURL)) ||
                    ((tempDOMString = getElementValue(controlURL)) == 0) ||
                    ((current->controlURL =
                          resolve_rel_url(URLBase, tempDOMString)) == 0)) {
                    UpnpPrintf(UPNP_INFO, GENA, __FILE__, __LINE__,
                               "BAD OR MISSING CONTROL URL");
                    UpnpPrintf(UPNP_INFO, GENA, __FILE__, __LINE__,
                               "CONTROL URL SET TO NULL IN "
                               "SERVICE INFO");
                    current->controlURL = NULL;
                    fail = 0;
                }
                ixmlFreeDOMString(tempDOMString);
                tempDOMString = NULL;
                if (!getSubElement("eventSubURL", current_service, &eventURL) ||
                    ((tempDOMString = getElementValue(eventURL)) == 0) ||
                    ((current->eventURL =
                          resolve_rel_url(URLBase, tempDOMString)) == 0)) {
                    UpnpPrintf(UPNP_INFO, GENA, __FILE__, __LINE__,
                               "BAD OR MISSING EVENT URL");
                    UpnpPrintf(UPNP_INFO, GENA, __FILE__, __LINE__,
                               "EVENT URL SET TO NULL IN "
                               "SERVICE INFO");
                    current->eventURL = NULL;
                    fail = 0;
                }
                ixmlFreeDOMString(tempDOMString);
                tempDOMString = NULL;
                if (fail) {
                    freeServiceList(current);
                    if (previous)
                        previous->next = NULL;
                    else
                        head = NULL;
                    current = previous;
                }
            }
            ixmlNodeList_free(serviceNodeList);
        }
        (*end) = current;
        return head;
    } else {
        (*end) = NULL;
        return NULL;
    }
}

/*!
 * \brief Returns pointer to service info after getting the sub-elements of the
 * service info.
 * \ingroup Eventing
 * \note Only available with the GENA module compiled in.
 *
 * \returns Service info.
 */
service_info* getAllServiceList(
    /*! [in] XML node information. */
    IXML_Node* node,
    /*! [in] provides Base URL to resolve relative URL. */
    char* URLBase,
    /*! [out] Service added is returned to the output parameter. */
    service_info** out_end) {
    service_info* head = NULL;
    service_info* end = NULL;
    service_info* next_end = NULL;
    IXML_NodeList* deviceList = NULL;
    IXML_Node* currentDevice = NULL;

    long unsigned int NumOfDevices = 0lu;
    long unsigned int i = 0lu;

    (*out_end) = NULL;
    deviceList =
        ixmlElement_getElementsByTagName((IXML_Element*)node, "device");
    if (deviceList) {
        NumOfDevices = ixmlNodeList_length(deviceList);
        for (i = 0lu; i < NumOfDevices; i++) {
            currentDevice = ixmlNodeList_item(deviceList, i);
            if (head) {
                end->next = getServiceList(currentDevice, &next_end, URLBase);
                if (next_end)
                    end = next_end;
            } else {
                head = getServiceList(currentDevice, &end, URLBase);
            }
        }
        ixmlNodeList_free(deviceList);
    }

    (*out_end) = end;
    return head;
}

} // anonymous namespace


int copy_subscription(subscription* in, subscription* out) {
    int return_code = HTTP_SUCCESS;

    memcpy(out->sid, in->sid, SID_SIZE);
    out->sid[SID_SIZE] = 0;
    out->ToSendEventKey = in->ToSendEventKey;
    out->expireTime = in->expireTime;
    out->active = in->active;
    return_code = copy_URL_list(&in->DeliveryURLs, &out->DeliveryURLs);
    if (return_code != HTTP_SUCCESS) {
        return return_code;
    }
    ListInit(&out->outgoing, 0, 0);
    out->next = NULL;
    return HTTP_SUCCESS;
}

void RemoveSubscriptionSID(Upnp_SID sid, service_info* service) {
    subscription* finger = service->subscriptionList;
    subscription* previous = NULL;

    while (finger) {
        if (!strcmp(sid, finger->sid)) {
            if (previous) {
                previous->next = finger->next;
            } else {
                service->subscriptionList = finger->next;
            }
            finger->next = NULL;
            freeSubscriptionList(finger);
            finger = NULL;
            service->TotalSubscriptions--;
        } else {
            previous = finger;
            finger = finger->next;
        }
    }
}

subscription* GetSubscriptionSID(const Upnp_SID sid, service_info* service) {
    subscription* next = service->subscriptionList;
    subscription* previous = NULL;
    subscription* found = NULL;
    time_t current_time;

    while (next && !found) {
        if (!strcmp(next->sid, sid))
            found = next;
        else {
            previous = next;
            next = next->next;
        }
    }
    if (found) {
        /* get the current_time */
        time(&current_time);
        if (found->expireTime && found->expireTime < current_time) {
            if (previous) {
                previous->next = found->next;
            } else {
                service->subscriptionList = found->next;
            }
            found->next = NULL;
            freeSubscriptionList(found);
            found = NULL;
            service->TotalSubscriptions--;
        }
    }
    return found;
}

subscription* GetFirstSubscription(service_info* service) {
    subscription temp;
    subscription* next = NULL;

    temp.next = service->subscriptionList;
    next = GetNextSubscription(service, &temp);
    service->subscriptionList = temp.next;
    /* service->subscriptionList = next; */

    return next;
}

subscription* GetNextSubscription(service_info* service,
                                  subscription* current) {
    time_t current_time;
    subscription* next = NULL;
    subscription* previous = NULL;
    int notDone = 1;

    /* get the current_time */
    time(&current_time);
    while (notDone && current) {
        previous = current;
        current = current->next;

        if (!current) {
            notDone = 0;
            next = current;
        } else if (current->expireTime && current->expireTime < current_time) {
            previous->next = current->next;
            current->next = NULL;
            freeSubscriptionList(current);
            current = previous;
            service->TotalSubscriptions--;
        } else if (current->active) {
            notDone = 0;
            next = current;
        }
    }
    return next;
}

void freeSubscription(subscription* sub) {
    if (sub) {
        free_URL_list(&sub->DeliveryURLs);
        freeSubscriptionQueuedEvents(sub);
    }
}

void freeSubscriptionList(subscription* head) {
    subscription* next = NULL;

    while (head) {
        next = head->next;
        freeSubscription(head);
        free(head);
        head = next;
    }
}

service_info* FindServiceId(service_table* table, const char* serviceId,
                            const char* UDN) {
    service_info* finger = NULL;

    if (table) {
        finger = table->serviceList;
        while (finger) {
            if (!strcmp(serviceId, finger->serviceId) &&
                !strcmp(UDN, finger->UDN)) {
                return finger;
            }
            finger = finger->next;
        }
    }

    return NULL;
}
#endif /* EXCLUDE_GENA */

service_info* FindServiceEventURLPath(service_table* table,
                                      const char* eventURLPath) {
    service_info* finger = NULL;
    uri_type parsed_url;
    uri_type parsed_url_in;

    if (!table || !eventURLPath) {
        return NULL;
    }
    if (parse_uri(eventURLPath, strlen(eventURLPath), &parsed_url_in) ==
        HTTP_SUCCESS) {
        finger = table->serviceList;
        while (finger) {
            if (finger->eventURL) {
                if (parse_uri(finger->eventURL, strlen(finger->eventURL),
                              &parsed_url) == HTTP_SUCCESS) {
                    if (!token_cmp(&parsed_url.pathquery,
                                   &parsed_url_in.pathquery)) {
                        return finger;
                    }
                }
            }
            finger = finger->next;
        }
    }

    return NULL;
}

#if (EXCLUDE_SOAP == 0) || defined(DOXYGEN_RUN)
service_info* FindServiceControlURLPath(service_table* table,
                                        const char* controlURLPath) {
    service_info* finger = NULL;
    uri_type parsed_url;
    uri_type parsed_url_in;

    if (!table || !controlURLPath) {
        return NULL;
    }
    if (parse_uri(controlURLPath, strlen(controlURLPath), &parsed_url_in) ==
        HTTP_SUCCESS) {
        finger = table->serviceList;
        while (finger) {
            if (finger->controlURL) {
                if (parse_uri(finger->controlURL, strlen(finger->controlURL),
                              &parsed_url) == HTTP_SUCCESS) {
                    if (!token_cmp(&parsed_url.pathquery,
                                   &parsed_url_in.pathquery)) {
                        return finger;
                    }
                }
            }
            finger = finger->next;
        }
    }

    return NULL;
}
#endif /* EXCLUDE_SOAP */

#if defined(DEBUG) || defined(DOXYGEN_RUN)
void printService(service_info* service, Upnp_LogLevel level,
                  Dbg_Module module) {
    if (service) {
        if (service->serviceType) {
            UpnpPrintf(level, module, __FILE__, __LINE__, "serviceType: %s\n",
                       service->serviceType);
        }
        if (service->serviceId) {
            UpnpPrintf(level, module, __FILE__, __LINE__, "serviceId: %s\n",
                       service->serviceId);
        }
        if (service->SCPDURL) {
            UpnpPrintf(level, module, __FILE__, __LINE__, "SCPDURL: %s\n",
                       service->SCPDURL);
        }
        if (service->controlURL) {
            UpnpPrintf(level, module, __FILE__, __LINE__, "controlURL: %s\n",
                       service->controlURL);
        }
        if (service->eventURL) {
            UpnpPrintf(level, module, __FILE__, __LINE__, "eventURL: %s\n",
                       service->eventURL);
        }
        if (service->UDN) {
            UpnpPrintf(level, module, __FILE__, __LINE__, "UDN: %s\n\n",
                       service->UDN);
        }
        if (service->active) {
            UpnpPrintf(level, module, __FILE__, __LINE__,
                       "Service is active\n");
        } else {
            UpnpPrintf(level, module, __FILE__, __LINE__,
                       "Service is inactive\n");
        }
    }
}
#endif

#if defined(DEBUG) || defined(DOXYGEN_RUN)
void printServiceList(service_info* service, Upnp_LogLevel level,
                      Dbg_Module module) {
    while (service) {
        if (service->serviceType) {
            UpnpPrintf(level, module, __FILE__, __LINE__, "serviceType: %s\n",
                       service->serviceType);
        }
        if (service->serviceId) {
            UpnpPrintf(level, module, __FILE__, __LINE__, "serviceId: %s\n",
                       service->serviceId);
        }
        if (service->SCPDURL) {
            UpnpPrintf(level, module, __FILE__, __LINE__, "SCPDURL: %s\n",
                       service->SCPDURL);
        }
        if (service->controlURL) {
            UpnpPrintf(level, module, __FILE__, __LINE__, "controlURL: %s\n",
                       service->controlURL);
        }
        if (service->eventURL) {
            UpnpPrintf(level, module, __FILE__, __LINE__, "eventURL: %s\n",
                       service->eventURL);
        }
        if (service->UDN) {
            UpnpPrintf(level, module, __FILE__, __LINE__, "UDN: %s\n\n",
                       service->UDN);
        }
        if (service->active) {
            UpnpPrintf(level, module, __FILE__, __LINE__,
                       "Service is active\n");
        } else {
            UpnpPrintf(level, module, __FILE__, __LINE__,
                       "Service is inactive\n");
        }
        service = service->next;
    }
}
#endif

#if defined(DEBUG) || defined(DOXYGEN_RUN)
void printServiceTable(service_table* table, Upnp_LogLevel level,
                       Dbg_Module module) {
    UpnpPrintf(level, module, __FILE__, __LINE__, "URL_BASE: %s\n",
               table->URLBase);
    UpnpPrintf(level, module, __FILE__, __LINE__, "Services: \n");
    printServiceList(table->serviceList, level, module);
}
#endif

#if (EXCLUDE_GENA == 0) || defined(DOXYGEN_RUN)
void freeService(service_info* in) {
    if (in) {
        if (in->serviceType)
            ixmlFreeDOMString(in->serviceType);

        if (in->serviceId)
            ixmlFreeDOMString(in->serviceId);

        if (in->SCPDURL)
            free(in->SCPDURL);

        if (in->controlURL)
            free(in->controlURL);

        if (in->eventURL)
            free(in->eventURL);

        if (in->UDN)
            ixmlFreeDOMString(in->UDN);

        if (in->subscriptionList)
            freeSubscriptionList(in->subscriptionList);

        in->TotalSubscriptions = 0;
        free(in);
    }
}

void freeServiceList(service_info* head) {
    service_info* next = NULL;

    while (head) {
        if (head->serviceType)
            ixmlFreeDOMString(head->serviceType);
        if (head->serviceId)
            ixmlFreeDOMString(head->serviceId);
        if (head->SCPDURL)
            free(head->SCPDURL);
        if (head->controlURL)
            free(head->controlURL);
        if (head->eventURL)
            free(head->eventURL);
        if (head->UDN)
            ixmlFreeDOMString(head->UDN);
        if (head->subscriptionList)
            freeSubscriptionList(head->subscriptionList);

        head->TotalSubscriptions = 0;
        next = head->next;
        free(head);
        head = next;
    }
}

void freeServiceTable(service_table* table) {
    ixmlFreeDOMString(table->URLBase);
    freeServiceList(table->serviceList);
    table->serviceList = NULL;
    table->endServiceList = NULL;
}

int removeServiceTable(IXML_Node* node, service_table* in) {
    IXML_Node* root = NULL;
    IXML_Node* currentUDN = NULL;
    DOMString UDN = NULL;
    IXML_NodeList* deviceList = NULL;
    service_info* current_service = NULL;
    service_info* start_search = NULL;
    service_info* prev_service = NULL;
    long unsigned int NumOfDevices = 0lu;
    long unsigned int i = 0lu;

    if (getSubElement("root", node, &root)) {
        start_search = in->serviceList;
        deviceList =
            ixmlElement_getElementsByTagName((IXML_Element*)root, "device");
        if (deviceList) {
            NumOfDevices = ixmlNodeList_length(deviceList);
            for (i = 0lu; i < NumOfDevices; i++) {
                if ((start_search) &&
                    ((getSubElement("UDN", node, &currentUDN)) &&
                     ((UDN = getElementValue(currentUDN)) != 0))) {
                    current_service = start_search;
                    /* Services are put in the service table
                     * in the order in which they appear in
                     * the description document, therefore
                     * we go through the list only once to
                     * remove a particular root device */
                    while ((current_service) &&
                           (strcmp(current_service->UDN, UDN))) {
                        current_service = current_service->next;
                        if (current_service != NULL)
                            prev_service = current_service->next;
                    }
                    while ((current_service) &&
                           (!strcmp(current_service->UDN, UDN))) {
                        if (prev_service) {
                            prev_service->next = current_service->next;
                        } else {
                            in->serviceList = current_service->next;
                        }
                        if (current_service == in->endServiceList)
                            in->endServiceList = prev_service;
                        start_search = current_service->next;
                        freeService(current_service);
                        current_service = start_search;
                    }
                    ixmlFreeDOMString(UDN);
                    UDN = NULL;
                }
            }

            ixmlNodeList_free(deviceList);
        }
    }
    return 1;
}

int addServiceTable(IXML_Node* node, service_table* in,
                    const char* DefaultURLBase) {
    IXML_Node* root = NULL;
    IXML_Node* URLBase = NULL;
    service_info* tempEnd = NULL;

    if (in->URLBase) {
        free(in->URLBase);
        in->URLBase = NULL;
    }
    if (getSubElement("root", node, &root)) {
        if (getSubElement("URLBase", root, &URLBase)) {
            in->URLBase = getElementValue(URLBase);
        } else {
            if (DefaultURLBase) {
                in->URLBase = ixmlCloneDOMString(DefaultURLBase);
            } else {
                in->URLBase = ixmlCloneDOMString("");
            }
        }
        if ((in->endServiceList->next =
                 getAllServiceList(root, in->URLBase, &tempEnd))) {
            in->endServiceList = tempEnd;
            return 1;
        }
    }

    return 0;
}

int getServiceTable(IXML_Node* node, service_table* out,
                    const char* DefaultURLBase) {
    IXML_Node* root = nullptr;
    IXML_Node* URLBase = nullptr;

    if (getSubElement("root", node, &root)) {
        if (getSubElement("URLBase", root, &URLBase)) {
            out->URLBase = getElementValue(URLBase);
        } else {
            if (DefaultURLBase) {
                out->URLBase = ixmlCloneDOMString(DefaultURLBase);
            } else {
                out->URLBase = ixmlCloneDOMString("");
            }
        }
        out->serviceList =
            getAllServiceList(root, out->URLBase, &out->endServiceList);
        if (out->serviceList) {
            return 1;
        }
    }

    return 0;
}

DOMString getElementValue(IXML_Node* node) {
    IXML_Node* child = (IXML_Node*)ixmlNode_getFirstChild(node);
    const DOMString temp = NULL;

    if (child && ixmlNode_getNodeType(child) == eTEXT_NODE) {
        temp = ixmlNode_getNodeValue(child);

        return ixmlCloneDOMString(temp);
    } else {
        return NULL;
    }
}

int getSubElement(const char* element_name, IXML_Node* node, IXML_Node** out) {
    const DOMString NodeName = NULL;
    int found = 0;
    IXML_Node* child = (IXML_Node*)ixmlNode_getFirstChild(node);

    (*out) = NULL;
    while (child && !found) {
        switch (ixmlNode_getNodeType(child)) {
        case eELEMENT_NODE:
            NodeName = ixmlNode_getNodeName(child);
            if (!strcmp(NodeName, element_name)) {
                (*out) = child;
                found = 1;
                return found;
            }
            break;
        default:
            break;
        }
        child = (IXML_Node*)ixmlNode_getNextSibling(child);
    }

    return found;
}

#endif /* EXCLUDE_GENA */

#endif /* INCLUDE_DEVICE_APIS */
