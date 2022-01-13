// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-01-11

#include "upnplib/httpparser.hpp"

namespace upnplib {

/************************************************************************
 * Function :	httpmsg_init
 *
 * Parameters :
 *	INOUT http_message_t* msg ;	HTTP Message Object
 *
 * Description :	Initialize and allocate memory for http message
 *
 * Return : void ;
 *
 * Note :
 ************************************************************************/
void httpmsg_init(http_message_t* msg) {
    msg->entity.buf = nullptr;
    msg->entity.length = (size_t)0;
    ListInit(&msg->headers, httpmsg_compare, httpheader_free);
    membuffer_init(&msg->msg);
    membuffer_init(&msg->status_msg);
    msg->urlbuf = nullptr;
    msg->initialized = 1;
}

} // namespace upnplib
