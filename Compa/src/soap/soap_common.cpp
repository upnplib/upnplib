// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft,  Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-02-01

#include <config.hpp>
#if (EXCLUDE_SOAP == 0) || defined(DOXYGEN_RUN)

#include <httpparser.hpp>
#include <sock.hpp>
#include <soaplib.hpp>

const char* ContentTypeHeader = "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n";

#endif /* EXCLUDE_SOAP */
