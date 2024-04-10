// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-04-11
/*!
 * \file
 * \brief Definations to manage the builtin Webserver
 */

#include <upnplib/webserver.hpp>
#include <upnplib/port.hpp>
/// \cond
#include <array>
/// \endcond

namespace {

/*!
 * \brief This table maps the file extension to the associated media type and
 * its subtype.
 *
 * Because we use effective binary search on this table it must be sorted by
 * file extension.
 */
const std::array<upnplib::Document_meta, 70> mediatype_list{
    {// file-ext, media-type, media-subtype
     {"aif", "audio", "aiff"},
     {"aifc", "audio", "aiff"},
     {"aiff", "audio", "aiff"},
     {"asf", "video", "x-ms-asf"},
     {"asx", "video", "x-ms-asf"},
     {"au", "audio", "basic"},
     {"avi", "video", "msvideo"},
     {"bmp", "image", "bmp"},
     {"css", "text", "css"},
     {"dcr", "application", "x-director"},
     {"dib", "image", "bmp"},
     {"dir", "application", "x-director"},
     {"dxr", "application", "x-director"},
     {"gif", "image", "gif"},
     {"hta", "text", "hta"},
     {"htm", "text", "html"},
     {"html", "text", "html"},
     {"jar", "application", "java-archive"},
     {"jfif", "image", "pjpeg"},
     {"jpe", "image", "jpeg"},
     {"jpeg", "image", "jpeg"},
     {"jpg", "image", "jpeg"},
     {"js", "application", "x-javascript"},
     {"kar", "audio", "midi"},
     {"m3u", "audio", "mpegurl"},
     {"mid", "audio", "midi"},
     {"midi", "audio", "midi"},
     {"mov", "video", "quicktime"},
     {"mp2v", "video", "x-mpeg2"},
     {"mp3", "audio", "mpeg"},
     {"mpe", "video", "mpeg"},
     {"mpeg", "video", "mpeg"},
     {"mpg", "video", "mpeg"},
     {"mpv", "video", "mpeg"},
     {"mpv2", "video", "x-mpeg2"},
     {"pdf", "application", "pdf"},
     {"pjp", "image", "jpeg"},
     {"pjpeg", "image", "jpeg"},
     {"plg", "text", "html"},
     {"pls", "audio", "scpls"},
     {"png", "image", "png"},
     {"qt", "video", "quicktime"},
     {"ram", "audio", "x-pn-realaudio"},
     {"rmi", "audio", "mid"},
     {"rmm", "audio", "x-pn-realaudio"},
     {"rtf", "application", "rtf"},
     {"shtml", "text", "html"},
     {"smf", "audio", "midi"},
     {"snd", "audio", "basic"},
     {"spl", "application", "futuresplash"},
     {"ssm", "application", "streamingmedia"},
     {"swf", "application", "x-shockwave-flash"},
     {"tar", "application", "tar"},
     {"tcl", "application", "x-tcl"},
     {"text", "text", "plain"},
     {"tif", "image", "tiff"},
     {"tiff", "image", "tiff"},
     {"txt", "text", "plain"},
     {"ulw", "audio", "basic"},
     {"wav", "audio", "wav"},
     {"wax", "audio", "x-ms-wax"},
     {"wm", "video", "x-ms-wm"},
     {"wma", "audio", "x-ms-wma"},
     {"wmv", "video", "x-ms-wmv"},
     {"wvx", "video", "x-ms-wvx"},
     {"xbm", "image", "x-xbitmap"},
     {"xml", "text", "xml"},
     {"xsl", "text", "xml"},
     {"z", "application", "x-compress"},
     {"zip", "application", "zip"}}};

} // anonymous namespace

namespace upnplib {

/// \todo Rework to use reference instead of pointer and use exception.
const Document_meta* select_filetype(std::string_view a_extension) {

    ssize_t top = 0;
    ssize_t bot = mediatype_list.size() - 1;

    // Using effective binary search on the sorted list.
    while (top <= bot) {
        ssize_t mid = (top + bot) / 2;
        int cmp = a_extension.compare(
            // need type cast: mid cannot become negative
            mediatype_list[(size_t)mid].extension);
        if (cmp > 0) {
            /* look below mid. */
            top = mid + 1;
        } else if (cmp < 0) {
            /* look above mid. */
            bot = mid - 1;
        } else {
            /* cmp == 0 */
            // need type cast: mid cannot become negative
            return &mediatype_list[(size_t)mid];
        }
    }
    return nullptr;
}

} // namespace upnplib
