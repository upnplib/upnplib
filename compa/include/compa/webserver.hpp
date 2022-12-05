#ifndef UPNPLIB_COMPA_WEBSERVER_HPP
#define UPNPLIB_COMPA_WEBSERVER_HPP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-11-11

struct xml_alias_t;

namespace compa {

/*!
 * \brief Replaces current alias with the given alias. To remove the current
 * alias, set alias_name to NULL.
 *
 * \note alias_content is not freed here
 *
 * \return
 * \li \c 0 - OK
 * \li \c UPNP_E_OUTOF_MEMORY
 */
UPNPLIB_API int web_server_set_alias(
    /*! [in] Webserver name of alias; created by caller and freed by caller
     * (doesn't even have to be malloc()d. */
    const char* alias_name,
    /*! [in] The xml doc; this is allocated by the caller; and freed by
     * the web server. */
    const char* alias_content,
    /*! [in] Length of alias body in bytes. */
    size_t alias_content_length,
    /*! [in] Time when the contents of alias were last changed (local time).
     */
    time_t last_modified);

/*!
 * \brief Based on the extension, returns the content type and content
 * subtype.
 *
 * \return
 * \li \c 0  on success
 * \li \c -1 if not found
 */
static UPNP_INLINE int search_extension(
    /*! [in] . */
    const char* a_extension,
    /*! [out] . */
    const char** a_con_type,
    /*! [out] . */
    const char** a_con_subtype);

/*!
 * \brief Check for the validity of the XML object buffer.
 *
 * \return int.
 */
static UPNP_INLINE bool is_valid_alias(
    /*! [in] XML alias object. */
    const struct xml_alias_t* alias);

/*!
 * \brief Release the XML document referred to by the input parameter. Free
 * the allocated buffers associated with this object.
 */
static void alias_release(
    /*! [in] XML alias object. */
    struct xml_alias_t* alias);

} // namespace compa

#endif // UPNPLIB_COMPA_WEBSERVER_HPP
