// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-01-25
/*!
 * \file
 * \brief Free functions to parse IPv6 and host URLs. Not usable, work in
 * progess.
 */

#include <upnplib/urlparser.hpp>

#include <string> // needed for MSC_VER
#include <iostream>

/// \cond
namespace upnplib {

/// \brief Parse IPv6 URL
// ======================
std::array<unsigned short, 8> ipv6_parser(std::string_view a_input) {
    std::array<unsigned short, 8> address{0, 0, 0, 0, 0, 0, 0, 0};

    unsigned short pieceIndex{0};
    int compress{-1};
    std::string_view::const_iterator pointer{a_input.begin()};
    int failure_line{0};

    unsigned char c = pointer < a_input.end() ? (unsigned char)*pointer : '\0';

    if (c == ':') {
        if (pointer + 1 >= a_input.end() ||
            *(pointer + 1) != (unsigned char)':') {
            failure_line = __LINE__;
            goto throw_failure;
        }

        pointer = pointer + 2;
        pieceIndex++;
        compress = pieceIndex;
    }

    while (pointer < a_input.end()) {
        if (pieceIndex >= 8) {
            failure_line = __LINE__;
            goto throw_failure;
        }

        if (*pointer == (unsigned char)':') {
            if (compress >= 0) {
                failure_line = __LINE__;
                goto throw_failure;
            }

            pointer++;
            pieceIndex++;
            compress = pieceIndex;

            continue;
        }

        unsigned short value{};
        int length{};
        while (length < 4 && pointer < a_input.end() &&
               std::isxdigit((unsigned char)*pointer)) {
            unsigned char hx = (unsigned char)*pointer;
            // hx interpreted as hexadecimal number
            unsigned short v =
                (hx >= 'A') ? (hx >= 'a') ? (hx - 'a' + 10) : (hx - 'A' + 10)
                            : (hx - '0');
            // Arithmetic with implicit type cast to int, type cast is checked
            value = (unsigned short)(value * 0x10 + v);
            pointer++;
            length++;
        }

        if (pointer < a_input.end() && *pointer == (unsigned char)'.') {
            if (length == 0) {
                failure_line = __LINE__;
                goto throw_failure;
            }

            pointer = pointer - length;

            if (pieceIndex > 6) {
                failure_line = __LINE__;
                goto throw_failure;
            }

            int numbersSeen{};
            while (pointer < a_input.end()) {

                // Valid values for ipv4Piece are 0 - 255
                // 65535 (signed -1) means uninitialized (NULL)
                unsigned short ipv4Piece{65535};
                if (numbersSeen > 0) {
                    if (*pointer == (unsigned char)'.' && numbersSeen < 4) {
                        pointer++;
                    } else {
                        failure_line = __LINE__;
                        goto throw_failure;
                    }
                }

                if (pointer < a_input.end() &&
                    !std::isdigit((unsigned char)*pointer)) {
                    failure_line = __LINE__;
                    goto throw_failure;
                }

                while (pointer < a_input.end() &&
                       std::isdigit((unsigned char)*pointer)) {
                    // interpreted as decimal number, type cast is checked
                    unsigned short number = (unsigned short)(*pointer - '0');
                    switch (ipv4Piece) {
                    case 65535: // means uninitialized (NULL)
                        ipv4Piece = number;
                        break;
                    case 0:
                        failure_line = __LINE__;
                        goto throw_failure;
                    default:
                        // Arithmetic with implicit type cast to int, type cast
                        // is checked
                        ipv4Piece = (unsigned short)(ipv4Piece * 10 + number);
                    }

                    if (ipv4Piece > 255) {
                        failure_line = __LINE__;
                        goto throw_failure;
                    }

                    pointer++;
                }

                // Arithmetic with implicit type cast to int
                address[pieceIndex] =
                    (unsigned short)(address[pieceIndex] * 0x100 + ipv4Piece);
                numbersSeen++;
                if (numbersSeen == 2 || numbersSeen == 4)
                    pieceIndex++;
            }

            if (numbersSeen != 4) {
                failure_line = __LINE__;
                goto throw_failure;
            }

            break;

        } else if (pointer < a_input.end() && *pointer == (unsigned char)':') {
            pointer++;
            if (pointer >= a_input.end()) {
                failure_line = __LINE__;
                goto throw_failure;
            }

        } else if (pointer < a_input.end()) {
            failure_line = __LINE__;
            goto throw_failure;
        }

        address[pieceIndex] = value;
        pieceIndex++;
    }

    if (compress >= 0) {
        int swaps{pieceIndex - compress};
        pieceIndex = 7;

        while (pieceIndex != 0 && swaps > 0) {
            // swap address[pieceIndex] with address[compress + swaps - 1]
            unsigned short swapIndex = (unsigned short)(compress + swaps - 1);
            unsigned short piece = address[pieceIndex];
            address[pieceIndex] = address[swapIndex];
            address[swapIndex] = piece;
            pieceIndex--;
            swaps--;
        }

    } else if (compress < 0 && pieceIndex != 8) {
        failure_line = __LINE__;
        goto throw_failure;
    }

    return address;

throw_failure:
    std::string errormsg =
        "Error: " + (std::string) __func__ + "(\"" + (std::string)a_input +
        "\"):" + std::to_string(failure_line) + " - Invalid IPv6 address.";
    std::clog << errormsg << std::endl;
    throw std::invalid_argument(errormsg);
}


/// \brief Parse host URL
// ======================
std::string host_parser(std::string_view a_input,
                        bool a_isNotSpecial = false) //
{
    if (!a_input.empty() && a_input.front() == '[') {
        if (a_input.back() != ']') {
            std::clog << "Error: missing closing ']'." << std::endl;
            throw std::invalid_argument("Missing closing ']': '" +
                                        (std::string)a_input + "'");
        }
        // Return the result of IPv6 parsing a_input with its leading U+005B ([)
        // and trailing U+005D (]) removed.
        return "2001:DB8:1234:5678:9ABC:DEF0:1234:5678";
    }

    if (a_isNotSpecial) {
        // If isNotSpecial is true, then return the result of opaque-host
        // parsing a_input.
        return "";
    }

    if (a_input.empty())
        throw std::invalid_argument("Host string must not be empty.");

    return "dummy.final.parsed.host";
}

} // namespace upnplib
/// \endcond
