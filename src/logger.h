/** @brief  Logger.
 *  @note   This is based on my On-Line MRC logger, which was based on
 *          this project's original logger. I don't prefix everything
 *          with 'easy' because I want this to be a relatively general-
 *          purpose, standalone file.
 *
 *  ## Change Log
 *  2024-10-11
 *  Added:
 *  - Optionally print new line
 *  - Rigorous error handling of errno
 *  - LOGGER_LEVEL_ALL and LOGGER_LEVEL_NONE
 */
#pragma once

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/// This is solely for readability. Otherwise, the meaning of a returned
/// '0' is not immediately clear.
#define LOGGER_OK (0)

// NOTE These are globally configurable parameters. Having these as macros
//      means there is no performance overhead for unused logging (assuming
//      effective dead code elimination).
#define LOGGER_STREAM stdout
#define LOGGER_LEVEL  LOGGER_LEVEL_NONE

// NOTE The relationship between these levels is subject to change. But
//      if you do go ahead and change them, you need to change the
//      EASY_LOGGER_LEVEL_STRINGS too.
enum LoggerLevels {
    LOGGER_LEVEL_ALL = INT_MIN,
    LOGGER_LEVEL_VERBOSE = 0,
    LOGGER_LEVEL_TRACE = 1,
    LOGGER_LEVEL_DEBUG = 2,
    LOGGER_LEVEL_INFO = 3,
    LOGGER_LEVEL_WARN = 4,
    LOGGER_LEVEL_ERROR = 5,
    LOGGER_LEVEL_FATAL = 6,
    LOGGER_LEVEL_NONE = INT_MAX,
};

static char const *const LOGGER_LEVEL_STRINGS[] =
    {"VERBOSE", "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

static inline int
_logger_header(FILE *const stream,
               enum LoggerLevels const threshold_level,
               enum LoggerLevels const log_level,
               int const errno_,
               char const *const file,
               int const line)
{
    int my_errno = 0;
    time_t t = {0};
    struct tm tm = {0};
    if (log_level < threshold_level) {
        return LOGGER_OK;
    }
    int const old_errno = errno;
    errno = 0;
    t = time(NULL);
    if (errno) {
        goto error_cleanup;
    }
    tm = *localtime(&t);
    if (errno) {
        goto error_cleanup;
    }
    fprintf(stream,
            "[%s] [%d-%02d-%02d %02d:%02d:%02d] [ %s:%d ] [errno %d: %s] ",
            LOGGER_LEVEL_STRINGS[log_level],
            tm.tm_year + 1900,
            tm.tm_mon + 1,
            tm.tm_mday,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec,
            file,
            line,
            // NOTE I print this even where there is no error to make
            //      the log easier to parse.
            errno_,
            strerror(errno_));
    if (errno) {
        goto error_cleanup;
    }
    errno = old_errno;
    return LOGGER_OK;
error_cleanup:
    errno = old_errno;
    return my_errno;
}

// NOTE I intentionally make all of the parameters verbose instead of
//      packing some of them into a structure. This is because then we
//      are not reliant on the structure if I decide I no longer like it.
/// @note   I capture errno outside of this function because any standard
///         library call (e.g. printf) can change the value of errno.
static inline int
_logger(FILE *const stream,
        enum LoggerLevels const threshold_level,
        enum LoggerLevels const log_level,
        int const errno_,
        char const *const file,
        int const line,
        bool const new_line,
        char const *const format,
        ...)
{
    int my_errno = 0;
    va_list ap;
    if (log_level < threshold_level) {
        return LOGGER_OK;
    }
    int const old_errno = errno;
    errno = 0;
    if ((my_errno = _logger_header(stream,
                                   threshold_level,
                                   log_level,
                                   errno_,
                                   file,
                                   line))) {
        goto error_cleanup;
    }
    va_start(ap, format);
    vfprintf(stream, format, ap);
    va_end(ap);
    if (new_line) {
        fprintf(stream, "\n");
    }
    fflush(stream);
    if (errno) {
        goto error_cleanup;
    }
    errno = old_errno;
    return LOGGER_OK;
error_cleanup:
    errno = old_errno;
    return my_errno;
}

#define LOGGER_VERBOSE(...)                                                    \
    _logger(LOGGER_STREAM,                                                     \
            LOGGER_LEVEL,                                                      \
            LOGGER_LEVEL_VERBOSE,                                              \
            errno,                                                             \
            __FILE__,                                                          \
            __LINE__,                                                          \
            true,                                                              \
            __VA_ARGS__)

#define LOGGER_TRACE(...)                                                      \
    _logger(LOGGER_STREAM,                                                     \
            LOGGER_LEVEL,                                                      \
            LOGGER_LEVEL_TRACE,                                                \
            errno,                                                             \
            __FILE__,                                                          \
            __LINE__,                                                          \
            true,                                                              \
            __VA_ARGS__)

#define LOGGER_DEBUG(...)                                                      \
    _logger(LOGGER_STREAM,                                                     \
            LOGGER_LEVEL,                                                      \
            LOGGER_LEVEL_DEBUG,                                                \
            errno,                                                             \
            __FILE__,                                                          \
            __LINE__,                                                          \
            true,                                                              \
            __VA_ARGS__)

#define LOGGER_INFO(...)                                                       \
    _logger(LOGGER_STREAM,                                                     \
            LOGGER_LEVEL,                                                      \
            LOGGER_LEVEL_INFO,                                                 \
            errno,                                                             \
            __FILE__,                                                          \
            __LINE__,                                                          \
            true,                                                              \
            __VA_ARGS__)

#define LOGGER_WARN(...)                                                       \
    _logger(LOGGER_STREAM,                                                     \
            LOGGER_LEVEL,                                                      \
            LOGGER_LEVEL_WARN,                                                 \
            errno,                                                             \
            __FILE__,                                                          \
            __LINE__,                                                          \
            true,                                                              \
            __VA_ARGS__)

#define LOGGER_ERROR(...)                                                      \
    _logger(LOGGER_STREAM,                                                     \
            LOGGER_LEVEL,                                                      \
            LOGGER_LEVEL_ERROR,                                                \
            errno,                                                             \
            __FILE__,                                                          \
            __LINE__,                                                          \
            true,                                                              \
            __VA_ARGS__)

#define LOGGER_FATAL(...)                                                      \
    _logger(LOGGER_STREAM,                                                     \
            LOGGER_LEVEL,                                                      \
            LOGGER_LEVEL_FATAL,                                                \
            errno,                                                             \
            __FILE__,                                                          \
            __LINE__,                                                          \
            true,                                                              \
            __VA_ARGS__)
