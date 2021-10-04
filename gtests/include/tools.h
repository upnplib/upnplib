// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-02

#ifndef UPNP_GTEST_TOOLS_H
#define UPNP_GTEST_TOOLS_H

#include <string>

const char* UpnpGetErrorMessage(int rc);

class CCaptureFd
// Tool to capture output to a file descriptor, mainly used to capture program
// output to stdout or stderr.
// When printing the captured output, all opened file descriptor will be closed
// to avoid confusing output loops. For a new capture after print(..) you have
// to call capture(..) again.
{
    int fd;
    int fd_old;
    int fd_log;
    bool err = true;
    std::string captFname;

  public:
    CCaptureFd();
    ~CCaptureFd();
    void capture(int prmFd);
    bool print(std::ostream& pOut);

  private:
    void closeFds();
};

#endif // UPNP_GTEST_TOOLS_H
