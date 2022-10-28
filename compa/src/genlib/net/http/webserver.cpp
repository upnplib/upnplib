// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-25

/*!
 * \file
 *
 * \brief Defines the Web Server and has functions to carry out
 * operations of the Web Server.
 */

#include "upnplib/port.hpp"
#include "upnplib/webserver.hpp"

namespace compa {

// This function do nothing. There is no media_list to initialize anymore with
// compatible code. It is only callable for compatibility.
static UPNPLIB_INLINE void media_list_init() {}

/*!
 * \brief Based on the extension, returns the content type and content
 * subtype.
 *
 * \return
 * \li \c 0  on success
 * \li \c -1 if not found
 */
static int search_extension(
    /*! [in] . */
    const char* a_extension,
    /*! [out] . */
    const char** a_con_type,
    /*! [out] . */
    const char** a_con_subtype) {

    const upnplib::Document_meta* filetype =
        upnplib::select_filetype(std::string_view(a_extension));

    if (filetype == nullptr) {
        return -1;
    }
    *a_con_type = filetype->type.c_str();
    *a_con_subtype = filetype->subtype.c_str();

    return 0;
}

} // namespace compa
