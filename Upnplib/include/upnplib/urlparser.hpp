// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-05-01
/*!
 * \file
 * \brief Free functions to parse IPv6 and host URLs. Not usable, work in
 * progess.
 * \cond
 * It isn't documented so far.
 */

// This class is based on the "URL Living Standard"
// ================================================
// At time the Url parser was coded this
// Commit Snapshot — Last Updated 21 February 2022 of the URL standard:
// https://url.spec.whatwg.org/commit-snapshots/9a83e251778046b20f4822f15ad4e2a469de2f57//
// was used.
//
// To understand the parser follow there the [basic URL parser]
// (https://url.spec.whatwg.org/#concept-basic-url-parse://url.spec.whatwg.org/commit-snapshots/9a83e251778046b20f4822f15ad4e2a469de2f57/#concept-basic-url-parser)
//
// To manual verify URLs conforming to the standard you can use the
// [Live URL Viewer](https://jsdom.github.io/whatwg-url/).

#ifndef UPNPLIB_NET_URI_URLPARSER_HPP
#define UPNPLIB_NET_URI_URLPARSER_HPP

#include <array>
#include <string_view>

namespace upnplib {

// IPv6 parser
// ===========
std::array<unsigned short, 8> ipv6_parser(std::string_view a_input);

} // namespace upnplib

/// \endcond

#endif // UPNPLIB_NET_URI_URLPARSER_HPP
