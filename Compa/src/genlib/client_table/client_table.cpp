// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-02-28
// Also Copyright by other contributor which haven't made a note.
// Last compare with pupnp original source file on 2023-06-22, ver 1.14.16
/*!
 * \file
 * \brief This file defines the functions for control points.
 *
 * It defines functions for adding and removing control points to and from the
 * client table, adding and accessing subscription and other attributes
 * pertaining to the control point.
 */

#include <config.hpp>
#include <client_table.hpp>

/// \cond
#include <cstdlib> /* for calloc(), free() */
/// \endcond

void free_client_subscription(GenlibClientSubscription* sub) {
    ThreadPoolJob tempJob;
    if (sub) {
        int renewEventId = GenlibClientSubscription_get_RenewEventId(sub);
        GenlibClientSubscription_strcpy_ActualSID(sub, "");
        GenlibClientSubscription_strcpy_EventURL(sub, "");
        if (renewEventId != -1) {
            /* do not remove timer event of copy */
            /* invalid timer event id */
            if (TimerThreadRemove(&gTimerThread, renewEventId, &tempJob) == 0) {
                tempJob.free_func(tempJob.arg);
            }
        }
        GenlibClientSubscription_set_RenewEventId(sub, -1);
    }
}

void freeClientSubList(GenlibClientSubscription* list) {
    GenlibClientSubscription* next;
    while (list) {
        free_client_subscription(list);
        next = GenlibClientSubscription_get_Next(list);
        GenlibClientSubscription_delete(list);
        list = next;
    }
}

void RemoveClientSubClientSID(GenlibClientSubscription** head,
                              const UpnpString* sid) {
    GenlibClientSubscription* finger = *head;
    GenlibClientSubscription* previous = NULL;
    int found = 0;
    while (finger) {
        found = !strcmp(UpnpString_get_String(sid),
                        GenlibClientSubscription_get_SID_cstr(finger));
        if (found) {
            if (previous) {
                GenlibClientSubscription_set_Next(
                    previous, GenlibClientSubscription_get_Next(finger));
            } else {
                *head = GenlibClientSubscription_get_Next(finger);
            }
            GenlibClientSubscription_set_Next(finger, NULL);
            freeClientSubList(finger);
            finger = NULL;
        } else {
            previous = finger;
            finger = GenlibClientSubscription_get_Next(finger);
        }
    }
}

GenlibClientSubscription* GetClientSubClientSID(GenlibClientSubscription* head,
                                                const UpnpString* sid) {
    GenlibClientSubscription* next = head;
    int found = 0;
    while (next) {
        found = !strcmp(GenlibClientSubscription_get_SID_cstr(next),
                        UpnpString_get_String(sid));
        if (found) {
            break;
        } else {
            next = GenlibClientSubscription_get_Next(next);
        }
    }

    return next;
}

GenlibClientSubscription* GetClientSubActualSID(GenlibClientSubscription* head,
                                                token* sid) {
    GenlibClientSubscription* next = head;
    while (next) {
        if (!memcmp(GenlibClientSubscription_get_ActualSID_cstr(next),
                    sid->buff, sid->size)) {
            break;
        } else {
            next = GenlibClientSubscription_get_Next(next);
        }
    }

    return next;
}
