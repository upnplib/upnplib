// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-11-11

/*!
 * \file
 *
 * \brief Defines the Web Server and has functions to carry out
 * operations of the Web Server.
 */

#include "upnplib/webserver.hpp"

namespace compa {

// This function do nothing. There is no media_list to initialize anymore with
// compatible code. It is only callable for compatibility.
static UPNP_INLINE void media_list_init() {}

//
static UPNP_INLINE int search_extension(
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

static UPNP_INLINE bool is_valid_alias(
    /*! [in] XML alias object. */
    const struct xml_alias_t* alias) {
    if (alias == nullptr)
        return false;
    return alias->ct != nullptr;
}

/*!
 * \brief Based on the extension, clones an XML string based on type and
 * content subtype. If content type and sub type are not found, unknown
 * types are used.
 *
 * \return
 * \li \c 0 on success.
 * \li \c UPNP_E_FILE_NOT_FOUND - on invalid filename.
 * \li \c UPNP_E_INVALID_ARGUMENT- on invalid fileInfo.
 * \li \c UPNP_E_OUTOF_MEMORY - on memory allocation failures.
 */
static UPNP_INLINE int get_content_type(
    /*! [in] . */
    const char* filename,
    /*! [out] . */
    UpnpFileInfo* fileInfo) {
    if (!filename)
        return UPNP_E_FILE_NOT_FOUND;
    if (!fileInfo)
        return UPNP_E_INVALID_ARGUMENT;

    const char* extension;
    const char* type;
    const char* subtype;
    int ctype_found = 0;
    char* temp = NULL;
    size_t length = 0;
    int rc = 0;

    UpnpFileInfo_set_ContentType(fileInfo, NULL);
    /* get ext */
    extension = strrchr(filename, '.');
    if (extension != NULL) {
        if (search_extension(extension + 1, &type, &subtype) == 0)
            ctype_found = 1;
    }
    if (!ctype_found) {
        /* unknown content type */
        type = gMediaTypes[APPLICATION_INDEX];
        subtype = "octet-stream";
    }
    length = strlen(type) + strlen("/") + strlen(subtype) + 1;
    temp = (char*)malloc(length);
    if (!temp)
        return UPNP_E_OUTOF_MEMORY;
    rc = snprintf(temp, length, "%s/%s", type, subtype);
    if (rc < 0 || (unsigned int)rc >= length) {
        free(temp);
        return UPNP_E_OUTOF_MEMORY;
    }
    UpnpFileInfo_set_ContentType(fileInfo, temp);
    free(temp);
    if (!UpnpFileInfo_get_ContentType(fileInfo))
        return UPNP_E_OUTOF_MEMORY;

    return 0;
}

/*!
 * \brief Release the XML document referred to by the input parameter. Free
 * the allocated buffers associated with this object.
 */
static void alias_release(
    /*! [in] XML alias object. */
    struct xml_alias_t* alias) {
    ithread_mutex_lock(&gWebMutex);
    /* ignore invalid alias */
    if (!NS::is_valid_alias(alias)) {
        ithread_mutex_unlock(&gWebMutex);
        return;
    }
    assert(*alias->ct > 0);
    *alias->ct -= 1;
    if (*alias->ct <= 0) {
        membuffer_destroy(&alias->doc);
        membuffer_destroy(&alias->name);
        umock::stdlib_h.free(alias->ct);
        alias->ct = nullptr;
        alias->last_modified = 0;
    }
    ithread_mutex_unlock(&gWebMutex);
}

} // namespace compa
