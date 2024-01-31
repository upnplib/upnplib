#ifndef COMPA_CLIENT_TABLE_HPP
#define COMPA_CLIENT_TABLE_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-01-29
// Last compare with pupnp original source file on 2023-06-22, ver 1.14.16
/*!
 * \file
 */

#include <GenlibClientSubscription.hpp>
#include <TimerThread.hpp>
#include <service_table.hpp>

extern TimerThread gTimerThread;

#if defined(INCLUDE_CLIENT_APIS) || defined(DOXYGEN_RUN)

/*!
 * \brief Free memory allocated for client subscription data.
 *
 * Remove timer thread associated with this subscription event.
 */
void free_client_subscription(
    /*! [in] Client subscription to be freed. */
    GenlibClientSubscription* sub);

/*!
 * \brief Free the client subscription table.
 */
void freeClientSubList(
    /*! [in] Client subscription list to be freed. */
    GenlibClientSubscription* list);

/*!
 * \brief Remove the client subscription matching the subscritpion id
 * represented by the const Upnp_SID sid parameter from the table and
 * update the table.
 */
void RemoveClientSubClientSID(
    /*! [in] Head of the subscription list. */
    GenlibClientSubscription** head,
    /*! [in] Subscription ID to be mactched. */
    const UpnpString* sid);

/*!
 * \brief Return the client subscription from the client table that matches
 * const Upnp_SID sid subscrition id value.
 *
 * \return The matching subscription.
 */
GenlibClientSubscription* GetClientSubClientSID(
    /*! [in] Head of the subscription list. */
    GenlibClientSubscription* head,
    /*! [in] Subscription ID to be mactched. */
    const UpnpString* sid);

/*!
 * \brief Returns the client subscription from the client subscription table
 * that has the matching token *sid buffer value.
 *
 * \return The matching subscription.
 */
GenlibClientSubscription* GetClientSubActualSID(
    /*! [in] Head of the subscription list. */
    GenlibClientSubscription* head,
    /*! [in] Subscription ID to be mactched. */
    token* sid);

#endif /* INCLUDE_CLIENT_APIS */

#endif /* COMPA_CLIENT_TABLE_HPP */
