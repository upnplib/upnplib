// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-08-24

// You cannot mock glibc mutex functions because they are also used by
// googletest. Googlemock will use a mocked function by itself. The test crashes
// with a segfault. But here are some tests how to check mutexes.
//
// For the structure of pthread_mutex_t type look at
// https://stackoverflow.com/q/23449508/5014688

#include "gtest/gtest.h"
#include <pthread.h>

TEST(MutexTestSuite, lock_and_unlock_mutex) {
    pthread_mutex_t fastmutex = PTHREAD_MUTEX_INITIALIZER;
    // pthread_mutex_t recmutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
    // pthread_mutex_t errchkmutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;

    // pthread_mutex_t init_mutex;   // together with
    // pthread_mutex_init(&init_mutex, NULL);

    EXPECT_EQ(fastmutex.__data.__lock, 0);
    std::cout << "DEBUG: init:   fastmutex.__data.__lock: "
              << fastmutex.__data.__lock
              << ", fastmutex.__data.__owner: " << fastmutex.__data.__owner
              << ", fastmutex.__data.__kind: " << fastmutex.__data.__kind
              << "\n";

    EXPECT_EQ(pthread_mutex_lock(&fastmutex), 0);
    EXPECT_EQ(pthread_mutex_destroy(&fastmutex), EBUSY);
    std::cout << "DEBUG: lock:   fastmutex.__data.__lock: "
              << fastmutex.__data.__lock
              << ", fastmutex.__data.__owner: " << fastmutex.__data.__owner
              << ", fastmutex.__data.__kind: " << fastmutex.__data.__kind
              << "\n";

    EXPECT_EQ(pthread_mutex_unlock(&fastmutex), 0);
    std::cout << "DEBUG: unlock: fastmutex.__data.__lock: "
              << fastmutex.__data.__lock
              << ", fastmutex.__data.__owner: " << fastmutex.__data.__owner
              << ", fastmutex.__data.__kind: " << fastmutex.__data.__kind
              << "\n";
    EXPECT_EQ(pthread_mutex_destroy(&fastmutex), 0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
