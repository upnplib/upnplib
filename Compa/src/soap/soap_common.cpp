// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft,  Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-02-14
/*! \file
 * \brief Common items for Control from a control point with SOAP.
 *
 * This is only available with SOAP enabled on compiling the library.
 */

#include <config.hpp>
#if (EXCLUDE_SOAP == 0) || defined(DOXYGEN_RUN)

#include <httpparser.hpp>
#include <soaplib.hpp>

const char* ContentTypeHeader = "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n";

#endif /* EXCLUDE_SOAP */
