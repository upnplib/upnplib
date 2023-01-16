// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-01-10

/*!
 * \file
 *
 * \brief Defines the Web Server and has functions to carry out
 * operations of the Web Server.
 */

#include "NS/webserver.hpp"
#include "upnplib/webserver.hpp"
#include "upnplib/port.hpp"

namespace compa {

struct xml_alias_t {
  public:
    /*! name of DOC from root; e.g.: /foo/bar/mydesc.xml */
    membuffer name{};
    /*! the XML document contents */
    membuffer doc{};
    /*! . */
    time_t last_modified{};
    /*! . */
    int* ct{nullptr};

    // Constructor
    xml_alias_t() {
        TRACE("construct compa::xml_alias_t\n");
        this->name.size_inc = 5;
        this->doc.size_inc = 5;
    }

    // Destructor
    ~xml_alias_t() {
        TRACE("destruct compa::xml_alias_t\n");
        // Free possible allocated membuffer.
        // this->clear(); // TODO: This will segfault.
    }

    // Methods
    int set(const char* a_alias_name, const char* a_alias_content,
            size_t a_alias_content_length,
            time_t a_last_modified = time(nullptr)) {
        TRACE("executing compa::xml_alias_t::set()\n");

        if (a_alias_content == nullptr ||
            (a_alias_name != nullptr && a_alias_content == this->doc.buf)) {
            TRACE("return compa::xml_alias_t::set() with "
                  "UPNP_E_INVALID_ARGUMENT\n");
            return UPNP_E_INVALID_ARGUMENT;
        }

        this->release();

        if (a_alias_name == nullptr) {
            /* don't serve aliased doc anymore */
            return UPNP_E_SUCCESS;
        }

        membuffer tmp_name{};
        tmp_name.size_inc = MEMBUF_DEF_SIZE_INC;
        membuffer tmp_doc{};
        tmp_doc.size_inc = MEMBUF_DEF_SIZE_INC;

        do {
            /* insert leading /, if missing */
            if (*a_alias_name != '/')
                if (membuffer_assign_str(&tmp_name, "/") != 0)
                    break; /* error; out of mem */
            if (membuffer_append_str(&tmp_name, a_alias_name) != 0)
                break; /* error */
            // a_alias_content_length must never exceed length of
            // a_alias_content.
            size_t content_len = strlen(a_alias_content);
            size_t doc_len = a_alias_content_length < content_len
                                 ? a_alias_content_length
                                 : content_len;
            membuffer_attach(&tmp_doc, (char*)a_alias_content, doc_len);
            tmp_doc.buf[doc_len] = '\0';
            // No errors, set properties
            TRACE("  set: allocate membuffer name\n");
            this->name = tmp_name;
            TRACE("  set: allocate membuffer doc\n");
            this->doc = tmp_doc;
            this->last_modified = a_last_modified;
            m_requested = 1;
            ct = &m_requested;

            return UPNP_E_SUCCESS;
        } while (0);
        /* error handler */
        /* free temp vars */
        TRACE("  set: already allocated membuffer name freed due to error\n");
        membuffer_destroy(&tmp_name);
        TRACE("  set: already allocated membuffer doc freed due to error\n");
        membuffer_destroy(&tmp_doc);

        TRACE("return compa::xml_alias_t::set() with "
              "UPNP_E_OUTOF_MEMORY\n");
        return UPNP_E_OUTOF_MEMORY;
    }

    bool is_valid() const { return m_requested > 0; }

    void release() {
        TRACE("executing compa::xml_alias_t::release()\n");
        int mutex_err = pthread_mutex_trylock(&gWebMutex);
        /* ignore invalid alias */
        if (m_requested > 0) {
            m_requested--;
            if (m_requested <= 0) {
                this->clear();
            } else {
                TRACE("  release: nothing released, more than one time "
                      "requested\n");
            }
        } else {
            TRACE("  release: nothing to release\n");
        }
        if (mutex_err == 0) {
            pthread_mutex_unlock(&gWebMutex);
        }
    }

    void clear() {
        TRACE("executing compa::xml_alias_t::clear()\n");
        int mutex_err = pthread_mutex_trylock(&gWebMutex);
        if (this->name.buf != nullptr) {
            TRACE("  clear: destroy membuffer name\n");
            membuffer_destroy(&this->name);
        }
        if (this->doc.buf != nullptr) {
            TRACE("  clear: destroy membuffer doc\n");
            membuffer_destroy(&this->doc);
        }
        this->last_modified = 0;
        this->ct = nullptr;
        m_requested = 0;
        if (mutex_err == 0) {
            pthread_mutex_unlock(&gWebMutex);
        }
    }

  private:
    int m_requested{};
};

static xml_alias_t gAliasDoc;

static UPNP_INLINE void glob_alias_init() {
    // This do nothing, Initialization is done by the constructor of gAliasDoc.
    // It's only available to emulate old code.
    TRACE("executing compa::glob_alias_init()\n");
}

/*!
 * \brief Check for the validity of the XML object buffer.
 *
 * \return bool.
 */
static UPNP_INLINE bool is_valid_alias(
    /*! [in] XML alias object. */
    const xml_alias_t* alias) {
    TRACE("executing compa::is_valid_alias()\n");
    if (alias == nullptr)
        return false;
    return alias->is_valid();
}

/*!
 * \brief Copy the contents of the global XML document into the local output
 * parameter.
 */
[[maybe_unused]] static void alias_grab(
    /*! [out] XML alias object. */
    xml_alias_t* a_alias) {
    TRACE("executing compa::alias_grab()\n");
    int mutex_err = pthread_mutex_trylock(&gWebMutex);
    if (a_alias != nullptr) {
        TRACE("  alias_grab: copy struct gAliasDoc\n");
        *a_alias = gAliasDoc;
        if (a_alias->ct != nullptr)
            *a_alias->ct = *a_alias->ct + 1;
    } else {
        TRACE("  alias_grab: nothing copied to nullptr\n");
    }
    if (mutex_err == 0)
        pthread_mutex_unlock(&gWebMutex);
}

/*!
 * \brief Release the XML document referred to by the input parameter. Free
 * the allocated buffers associated with this object.
 */
[[maybe_unused]] static void alias_release(
    /*! [in] XML alias object. */
    xml_alias_t* a_alias) {
    TRACE("executing compa::alias_release()\n");
    if (a_alias != nullptr)
        a_alias->release();
}

//
int web_server_set_alias(const char* alias_name, const char* alias_content,
                         size_t alias_content_length, time_t last_modified) {
    TRACE("executing compa::web_server_set_alias()\n");
    return gAliasDoc.set(alias_name, alias_content, alias_content_length,
                         last_modified);
}

// This function do nothing. There is no media_list to initialize anymore with
// compatible code. It is only callable for compatibility.
static UPNP_INLINE void media_list_init() {
    TRACE("executing compa::media_list_init()\n");
}

//
static UPNP_INLINE int search_extension(
    /*! [in] . */
    const char* a_extension,
    /*! [out] . */
    const char** a_con_type,
    /*! [out] . */
    const char** a_con_subtype) {
    TRACE("executing compa::search_extension()\n");

    const upnplib::Document_meta* filetype =
        upnplib::select_filetype(std::string_view(a_extension));

    if (filetype == nullptr) {
        TRACE("  search_extension: return with -1\n");
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
    const char* type{};
    const char* subtype{};
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
