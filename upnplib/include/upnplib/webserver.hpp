#ifndef UPNPLIB_WEBSERVER_HPP
#define UPNPLIB_WEBSERVER_HPP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-12-03

#include "upnplib/visibility.hpp"
#include <string>

namespace upnplib {

/* mapping of file extension to content-type of document */
struct Document_meta {
    /*! . */
    std::string extension;
    /*! . */
    std::string type;
    /*! . */
    std::string subtype;
};

/*!
 * \brief Based on the extension, returns the content type and content
 * subtype.
 *
 * \return
 * \li \c pointer to a content type structure for a known file extension
 * \li \c nullptr if file extension was not known
 *
 * \example
 *  #include "upnplib/webserver.hpp"
 *  #include <iostream>
 *  const Document_meta* doc_meta = select_filetype("mp3");
 *  if (doc_meta != nullptr) {
 *      std::cout << "type = " << doc_meta->type
 *                << ", subtype = " << doc_meta->subtype << "\n";
 *  }
 */
const Document_meta* select_filetype(
    /*! [in] . */
    std::string_view a_extension);

} // namespace upnplib

#endif // UPNPLIB_WEBSERVER_HPP
