/* upnp/sample/common/config_sample.h.  Generated from config_sample.h.in by
 * configure.  */
/*
 * On systems where the size of off_t depends on compile flags, libupnp needs
 * the programs which use it to be built with the same options as itself.
 * This is checked at compile time, but the actual variables (such as
 * _FILE_OFFSET_BITS) are not exported by the library (to avoid surprises in
 * applications).
 * A "normal" application would need to explicitely configure large file
 * support depending on how libupnp was built, using its own configure
 * routines. For the samples, we equivalently let the main libupnp configure
 * create this file.
 */

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */
