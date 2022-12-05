// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-12-08

/*!
 * \file
 *
 * \brief Defines the Web Server and has functions to carry out
 * operations of the Web Server.
 */

#include "NS/webserver.hpp"
#include "upnplib/webserver.hpp"

namespace compa {

static UPNP_INLINE bool is_valid_alias(
    /*! [in] XML alias object. */
    const struct xml_alias_t* alias) {
    if (alias == nullptr)
        return false;
    return alias->ct != nullptr;
}

/*!
 * \brief Release the XML document referred to by the input parameter. Free
 * the allocated buffers associated with this object.
 */
[[maybe_unused]] static void alias_release(
    /*! [in] XML alias object. */
    struct xml_alias_t* alias) {
    ithread_mutex_lock(&gWebMutex);
    /* ignore invalid alias */
    if (!compa::is_valid_alias(alias)) {
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

//
int web_server_set_alias(const char* alias_name, const char* alias_content,
                         size_t alias_content_length, time_t last_modified) {
    int ret_code;
    struct xml_alias_t alias;

    compa::alias_release(&gAliasDoc);
    if (alias_name == nullptr) {
        /* don't serve aliased doc anymore */
        return 0;
    }
    if (alias_content == nullptr) {
        return UPNP_E_INVALID_ARGUMENT;
    }
    membuffer_init(&alias.doc);
    membuffer_init(&alias.name);
    alias.ct = nullptr;
    do {
        /* insert leading /, if missing */
        if (*alias_name != '/')
            if (membuffer_assign_str(&alias.name, "/") != 0)
                break; /* error; out of mem */
        ret_code = membuffer_append_str(&alias.name, alias_name);
        if (ret_code != 0)
            break; /* error */
        if ((alias.ct = (int*)malloc(sizeof(int))) == NULL)
            break; /* error */
        *alias.ct = 1;
        if (alias_content_length) {
            membuffer_attach(&alias.doc, (char*)alias_content,
                             alias_content_length);
        }
        alias.last_modified = last_modified;
        /* save in module var */
        ithread_mutex_lock(&gWebMutex);
        gAliasDoc = alias;
        ithread_mutex_unlock(&gWebMutex);

        return 0;
    } while (0);
    /* error handler */
    /* free temp alias */
    membuffer_destroy(&alias.name);
    membuffer_destroy(&alias.doc);
    umock::stdlib_h.free(alias.ct);

    return UPNP_E_OUTOF_MEMORY;
}

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

} // namespace compa
