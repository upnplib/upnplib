# Platform specific libraries OpenSSL
Don't change the directory names. They are used by CMake with variable
${CMAKE_HOST_SYSTEM_NAME}.

Using SSL/TLS security with nowadays default https communication, OpenSSL is
very important for this project. To simplify build and to be independent from
platform specific openssl availability and installations I use precompiled
OpenSSL libraries. They are part of the project so we do not have to worry
about to find the paths to an OpenSSL system wide installation that are
different on every platform and mostly needs to be managed by the
user/developer.

There is a workflow 'Get_openssl_libs.yml' that runs on Github Actions test
images 'ubuntu-latest', 'macos-latest' and 'windows-latest'. It creats
artefacts containing just the runtime OpenSSL libraries from the platform. As
usual these artefacts can be downloaded.

Importing and configuring the libraries to be usable with CMake is done in
${UPNPLIB_PROJECT_SOURCE_DIR}/openssl/CMakeLists.txt.

## Platform Linux:
OpenSSL static libraries version 1.1.1, taken from Github Action test image
'ubuntu-latest' on 2023-02-16. Header files are used from the actual build on
2023-02-19 for Microsoft Windows.

## Platform Darwin (MacOS);
OpenSSL static libraries version 1.1.1, taken from Github Action test image
'macos-latest' on 2023-02-16. Header files are used from the actual build on
2023-02-19 for Microsoft Windows.

## Platform Microsoft Windows:
OpenSSL shared libraries version 3.0.8, cloned from
https://github.com/openssl/openssl and build on 2023-02-19 on a Microsoft
Windows 10 platform. I wasn't able to make the binaries from the Github Action
running, so I decided to build them from the OpenSSL project. The header files
from this build seem to be downstream compatible so I use them also for the
other platforms. Trying to use the static libraries ends up in incompatibility
with system libraries. The linker expects shared libraries or not default
linker options.

## Create self signed certificates for testing
I use this command to create them with a validity of 10 years (-days 3650):

    $ openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 3650 -nodes

Decode the certificate to show its information:

    $ openssl x509 -in cert.pem -noout -text

The test environment expects the certificates in ${UPNPLIB_PROJECT_SOURCE_DIR}/gtests.

<pre><sup>
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, &#60;Ingo&#64;Hoeft-online.de&#62;
// Redistribution only with this Copyright remark. Last modified: 2023-03-08
</sup></sup>
