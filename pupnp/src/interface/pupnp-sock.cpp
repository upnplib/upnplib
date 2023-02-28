// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-03-03

#include <interface/pupnp-sock.hpp>

namespace pupnp {

// Interface for the sock module
// =============================

// This destructor can also be defined direct in the header file as usual but
// then the symbol is included and not linked. We have to decorate the symbol
// with __declspec(dllexport) on Microsoft Windows in a header file that
// normaly import symbols. It does not conform to the visibility macro
// UPNPLIB_API and would require other advanced special handling. So we link
// the symbol with this source file so it do the right think with it. --Ingo
SockInterface::~SockInterface() = default;

} // namespace pupnp
