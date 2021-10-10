// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-03

// Tools and helper classes to manage gtests
// =========================================

#include "tools.h"
#include "port.hpp"
#include "upnp.h" // for UPNP_E_* constants

#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include PORT_UNISTD_H

// Errormessages taken from https://github.com/pupnp/pupnp
// Author: Marcelo Roberto Jimenez <marcelo.jimenez@gmail.com>
//------------------------------------------------------------
/*!
 * \brief Structure to maintain a error code and string associated with the
 * error code.
 */
struct ErrorString {
    /*! Error code. */
    int rc;
    /*! Error description. */
    const char* rcError;
};

/*!
 * \brief Array of error structures.
 */
struct ErrorString ErrorMessages[] = {
    {UPNP_E_SUCCESS, "UPNP_E_SUCCESS"},
    {UPNP_E_INVALID_HANDLE, "UPNP_E_INVALID_HANDLE"},
    {UPNP_E_INVALID_PARAM, "UPNP_E_INVALID_PARAM"},
    {UPNP_E_OUTOF_HANDLE, "UPNP_E_OUTOF_HANDLE"},
    {UPNP_E_OUTOF_CONTEXT, "UPNP_E_OUTOF_CONTEXT"},
    {UPNP_E_OUTOF_MEMORY, "UPNP_E_OUTOF_MEMORY"},
    {UPNP_E_INIT, "UPNP_E_INIT"},
    {UPNP_E_BUFFER_TOO_SMALL, "UPNP_E_BUFFER_TOO_SMALL"},
    {UPNP_E_INVALID_DESC, "UPNP_E_INVALID_DESC"},
    {UPNP_E_INVALID_URL, "UPNP_E_INVALID_URL"},
    {UPNP_E_INVALID_SID, "UPNP_E_INVALID_SID"},
    {UPNP_E_INVALID_DEVICE, "UPNP_E_INVALID_DEVICE"},
    {UPNP_E_INVALID_SERVICE, "UPNP_E_INVALID_SERVICE"},
    {UPNP_E_BAD_RESPONSE, "UPNP_E_BAD_RESPONSE"},
    {UPNP_E_BAD_REQUEST, "UPNP_E_BAD_REQUEST"},
    {UPNP_E_INVALID_ACTION, "UPNP_E_INVALID_ACTION"},
    {UPNP_E_FINISH, "UPNP_E_FINISH"},
    {UPNP_E_INIT_FAILED, "UPNP_E_INIT_FAILED"},
    {UPNP_E_URL_TOO_BIG, "UPNP_E_URL_TOO_BIG"},
    {UPNP_E_BAD_HTTPMSG, "UPNP_E_BAD_HTTPMSG"},
    {UPNP_E_ALREADY_REGISTERED, "UPNP_E_ALREADY_REGISTERED"},
    {UPNP_E_INVALID_INTERFACE, "UPNP_E_INVALID_INTERFACE"},
    {UPNP_E_NETWORK_ERROR, "UPNP_E_NETWORK_ERROR"},
    {UPNP_E_SOCKET_WRITE, "UPNP_E_SOCKET_WRITE"},
    {UPNP_E_SOCKET_READ, "UPNP_E_SOCKET_READ"},
    {UPNP_E_SOCKET_BIND, "UPNP_E_SOCKET_BIND"},
    {UPNP_E_SOCKET_CONNECT, "UPNP_E_SOCKET_CONNECT"},
    {UPNP_E_OUTOF_SOCKET, "UPNP_E_OUTOF_SOCKET"},
    {UPNP_E_LISTEN, "UPNP_E_LISTEN"},
    {UPNP_E_TIMEDOUT, "UPNP_E_TIMEDOUT"},
    {UPNP_E_SOCKET_ERROR, "UPNP_E_SOCKET_ERROR"},
    {UPNP_E_FILE_WRITE_ERROR, "UPNP_E_FILE_WRITE_ERROR"},
    {UPNP_E_CANCELED, "UPNP_E_CANCELED"},
    {UPNP_E_EVENT_PROTOCOL, "UPNP_E_EVENT_PROTOCOL"},
    {UPNP_E_SUBSCRIBE_UNACCEPTED, "UPNP_E_SUBSCRIBE_UNACCEPTED"},
    {UPNP_E_UNSUBSCRIBE_UNACCEPTED, "UPNP_E_UNSUBSCRIBE_UNACCEPTED"},
    {UPNP_E_NOTIFY_UNACCEPTED, "UPNP_E_NOTIFY_UNACCEPTED"},
    {UPNP_E_INVALID_ARGUMENT, "UPNP_E_INVALID_ARGUMENT"},
    {UPNP_E_FILE_NOT_FOUND, "UPNP_E_FILE_NOT_FOUND"},
    {UPNP_E_FILE_READ_ERROR, "UPNP_E_FILE_READ_ERROR"},
    {UPNP_E_EXT_NOT_XML, "UPNP_E_EXT_NOT_XML"},
    {UPNP_E_NO_WEB_SERVER, "UPNP_E_NO_WEB_SERVER"},
    {UPNP_E_OUTOF_BOUNDS, "UPNP_E_OUTOF_BOUNDS"},
    {UPNP_E_NOT_FOUND, "UPNP_E_NOT_FOUND"},
    {UPNP_E_INTERNAL_ERROR, "UPNP_E_INTERNAL_ERROR"},
};

const char* UpnpGetErrorMessage(int rc) {
    size_t i;

    for (i = 0; i < sizeof(ErrorMessages) / sizeof(ErrorMessages[0]); ++i) {
        if (rc == ErrorMessages[i].rc) {
            return ErrorMessages[i].rcError;
        }
    }

    return "Unknown error code";
}

CCaptureFd::CCaptureFd() {
    // generate random temporary filename to be thread-safe
    std::srand(std::time(nullptr));
    this->captFname = std::filesystem::temp_directory_path().string() +
                      "/gtestcapt" + std::to_string(std::rand());
}

CCaptureFd::~CCaptureFd() {
    this->closeFds();
    remove(this->captFname.c_str());
}

void CCaptureFd::capture(int prmFd) {
    this->fd = prmFd;
    this->fd_old = ::dup(prmFd);
    if (this->fd_old < 0) {
        return;
    }
    this->fd_log =
        ::open(this->captFname.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0660);
    if (this->fd_log < 0) {
        ::close(this->fd_old);
        return;
    }
    if (::dup2(this->fd_log, prmFd) < -2) {
        ::close(this->fd_old);
        ::close(this->fd_log);
        return;
    }
    this->err = false;
}

bool CCaptureFd::print(std::ostream& pOut)
// Close all file descriptors and print captured file content.
// If nothing was captured, then the return value is false.
{
    if (this->err)
        return false;
    this->closeFds();

    std::ifstream readFileObj(this->captFname.c_str());
    std::string lineBuf = "";

    std::getline(readFileObj, lineBuf);
    if (lineBuf == "") {
        readFileObj.close();
        remove(this->captFname.c_str());
        return false;
    }

    pOut << lineBuf << "\n";
    while (std::getline(readFileObj, lineBuf))
        pOut << lineBuf << "\n";

    readFileObj.close();
    remove(this->captFname.c_str());
    return true;
}

void CCaptureFd::closeFds() {
    // restore old fd
    ::dup2(this->fd_old, this->fd);
    ::close(this->fd_old);
    ::close(this->fd_log);
    this->err = true;
}
