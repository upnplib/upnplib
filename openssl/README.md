Platform specific static libraries OpenSSL
==========================================
All variants are version 1.1.1.

Using SSL/TLS security with nowadays default https communication, OpenSSL is
very important for this project. To simplify build and to be independent from
platform specific openssl availability and installations I use precompiled
OpenSSL static libraries. They are part of the project so we do not have to
worry about to find the paths to an OpenSSL system wide installation that are
different on every platform and mostly needs to be managed by the user.

The files have been downloaded from Github Actions test images 'ubuntu-latest',
'macos-latest' and 'windows-latest' on 2023-02-16. There is workflow
'Get_openssl_libs.yml' that creates Github artefacts which can be downloaded.

The header files in include/openssl/ are taken from the MacOS build and
commonly used for all platforms. The headers from the Ubuntu build strangely
enough does not compile with this projects environment.
