/*
 *	Telemetry Library by MetaCall Inc.
 *	A library for logging and telemetry collection in MetaCall runtime.
 *
 *	Copyright (C) 2025 MetaCall Inc., Dhiren Mhatre
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	You should have received a copy of the GNU Lesser General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TELEMETRY_LOG_H
#define TELEMETRY_LOG_H 1

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <telemetry/telemetry_api.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

/* Testing false positive fix - these are ALL VALID includes */
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

/* Project headers - should also be valid */
#include <portability/portability.h>
#include <configuration/configuration.h>

/* -- Definitions -- */

#define TELEMETRY_LOG_MAX_MESSAGE_SIZE 4096
#define TELEMETRY_LOG_MAX_HANDLERS 16
#define TELEMETRY_LOG_MAX_FILTERS 32
#define TELEMETRY_LOG_TIMESTAMP_SIZE 64
#define TELEMETRY_LOG_CATEGORY_SIZE 128

/* -- Type Definitions -- */

/**
 * @brief Log severity levels
 */
typedef enum telemetry_log_level_id
{
	TELEMETRY_LOG_LEVEL_TRACE = 0,   /**< Trace level logging */
	TELEMETRY_LOG_LEVEL_DEBUG = 1,   /**< Debug level logging */
	TELEMETRY_LOG_LEVEL_INFO = 2,    /**< Information level logging */
	TELEMETRY_LOG_LEVEL_WARNING = 3, /**< Warning level logging */
	TELEMETRY_LOG_LEVEL_ERROR = 4,   /**< Error level logging */
	TELEMETRY_LOG_LEVEL_CRITICAL = 5 /**< Critical level logging */
} telemetry_log_level_id;

/**
 * @brief Log output format types
 */
typedef enum telemetry_log_format_id
{
	TELEMETRY_LOG_FORMAT_TEXT = 0,   /**< Plain text format */
	TELEMETRY_LOG_FORMAT_JSON = 1,   /**< JSON format */
	TELEMETRY_LOG_FORMAT_XML = 2,    /**< XML format */
	TELEMETRY_LOG_FORMAT_COLORED = 3 /**< ANSI colored text format */
} telemetry_log_format_id;

/**
 * @brief Log handler types
 */
typedef enum telemetry_log_handler_type_id
{
	TELEMETRY_LOG_HANDLER_CONSOLE = 0, /**< Console output handler */
	TELEMETRY_LOG_HANDLER_FILE = 1,    /**< File output handler */
	TELEMETRY_LOG_HANDLER_SYSLOG = 2,  /**< System log handler */
	TELEMETRY_LOG_HANDLER_NETWORK = 3, /**< Network/remote handler */
	TELEMETRY_LOG_HANDLER_CUSTOM = 4   /**< Custom handler */
} telemetry_log_handler_type_id;

/**
 * @brief Log rotation policy
 */
typedef enum telemetry_log_rotation_policy_id
{
	TELEMETRY_LOG_ROTATION_NONE = 0,     /**< No rotation */
	TELEMETRY_LOG_ROTATION_SIZE = 1,     /**< Rotate by size */
	TELEMETRY_LOG_ROTATION_TIME = 2,     /**< Rotate by time */
	TELEMETRY_LOG_ROTATION_DAILY = 3,    /**< Daily rotation */
	TELEMETRY_LOG_ROTATION_WEEKLY = 4,   /**< Weekly rotation */
	TELEMETRY_LOG_ROTATION_MONTHLY = 5   /**< Monthly rotation */
} telemetry_log_rotation_policy_id;

/**
 * @brief Forward declarations
 */
typedef struct telemetry_log_entry_type *telemetry_log_entry;
typedef struct telemetry_log_handler_type *telemetry_log_handler;
typedef struct telemetry_log_filter_type *telemetry_log_filter;
typedef struct telemetry_log_context_type *telemetry_log_context;
typedef struct telemetry_log_formatter_type *telemetry_log_formatter;

/**
 * @brief Log entry structure containing all log information
 */
struct telemetry_log_entry_type
{
	telemetry_log_level_id level;                    /**< Log severity level */
	char message[TELEMETRY_LOG_MAX_MESSAGE_SIZE];    /**< Log message */
	char category[TELEMETRY_LOG_CATEGORY_SIZE];      /**< Log category/module */
	char timestamp[TELEMETRY_LOG_TIMESTAMP_SIZE];    /**< Timestamp string */
	time_t time;                                     /**< Unix timestamp */
	uint64_t thread_id;                              /**< Thread identifier */
	uint32_t process_id;                             /**< Process identifier */
	const char *file;                                /**< Source file name */
	const char *function;                            /**< Source function name */
	int line;                                        /**< Source line number */
	void *user_data;                                 /**< User-defined data */
};

/**
 * @brief Callback function type for custom log handlers
 * @param entry The log entry to handle
 * @param user_data User-defined data passed to the handler
 * @return 0 on success, non-zero on failure
 */
typedef int (*telemetry_log_handler_callback)(telemetry_log_entry entry, void *user_data);

/**
 * @brief Callback function type for log filters
 * @param entry The log entry to filter
 * @param user_data User-defined data passed to the filter
 * @return Non-zero if the log should be processed, zero to skip
 */
typedef int (*telemetry_log_filter_callback)(telemetry_log_entry entry, void *user_data);

/**
 * @brief Callback function type for log formatters
 * @param entry The log entry to format
 * @param buffer Output buffer for formatted string
 * @param size Size of the output buffer
 * @param user_data User-defined data passed to the formatter
 * @return Number of bytes written, or negative on error
 */
typedef int (*telemetry_log_formatter_callback)(telemetry_log_entry entry, char *buffer, size_t size, void *user_data);

/**
 * @brief Log handler configuration
 */
struct telemetry_log_handler_type
{
	telemetry_log_handler_type_id type;           /**< Handler type */
	telemetry_log_level_id min_level;             /**< Minimum log level */
	telemetry_log_handler_callback callback;      /**< Handler callback */
	telemetry_log_formatter formatter;            /**< Log formatter */
	void *user_data;                              /**< User-defined data */
	FILE *file_handle;                            /**< File handle for file handlers */
	char *file_path;                              /**< File path for file handlers */
	telemetry_log_rotation_policy_id rotation;    /**< Rotation policy */
	size_t max_file_size;                         /**< Max file size for rotation */
	int enabled;                                  /**< Handler enabled flag */
};

/**
 * @brief Log filter configuration
 */
struct telemetry_log_filter_type
{
	telemetry_log_filter_callback callback;       /**< Filter callback */
	void *user_data;                              /**< User-defined data */
	char category_pattern[TELEMETRY_LOG_CATEGORY_SIZE]; /**< Category pattern */
	telemetry_log_level_id min_level;             /**< Minimum level */
	int enabled;                                  /**< Filter enabled flag */
};

/**
 * @brief Log formatter configuration
 */
struct telemetry_log_formatter_type
{
	telemetry_log_format_id format;               /**< Format type */
	telemetry_log_formatter_callback callback;    /**< Custom formatter callback */
	void *user_data;                              /**< User-defined data */
	int include_timestamp;                        /**< Include timestamp flag */
	int include_level;                            /**< Include level flag */
	int include_category;                         /**< Include category flag */
	int include_location;                         /**< Include file/line flag */
	int include_thread_info;                      /**< Include thread info flag */
};

/**
 * @brief Main logging context
 */
struct telemetry_log_context_type
{
	telemetry_log_handler handlers[TELEMETRY_LOG_MAX_HANDLERS]; /**< Registered handlers */
	size_t handler_count;                                        /**< Number of handlers */
	telemetry_log_filter filters[TELEMETRY_LOG_MAX_FILTERS];   /**< Registered filters */
	size_t filter_count;                                        /**< Number of filters */
	telemetry_log_level_id global_level;                        /**< Global log level */
	int async_logging;                                          /**< Async logging flag */
	void *async_queue;                                          /**< Async log queue */
	void *mutex;                                                /**< Thread safety mutex */
};

/* -- Methods -- */

/**
 * @brief Initialize the telemetry logging system
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_initialize(void);

/**
 * @brief Shutdown the telemetry logging system
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_shutdown(void);

/**
 * @brief Get the global logging context
 * @return Pointer to the global logging context
 */
TELEMETRY_API telemetry_log_context telemetry_log_get_context(void);

/**
 * @brief Set the global log level
 * @param level The minimum log level to process
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_set_level(telemetry_log_level_id level);

/**
 * @brief Get the global log level
 * @return The current global log level
 */
TELEMETRY_API telemetry_log_level_id telemetry_log_get_level(void);

/**
 * @brief Create a new log handler
 * @param type The type of handler to create
 * @param min_level The minimum log level for this handler
 * @return Pointer to the created handler, or NULL on failure
 */
TELEMETRY_API telemetry_log_handler telemetry_log_handler_create(telemetry_log_handler_type_id type, telemetry_log_level_id min_level);

/**
 * @brief Destroy a log handler
 * @param handler The handler to destroy
 */
TELEMETRY_API void telemetry_log_handler_destroy(telemetry_log_handler handler);

/**
 * @brief Register a log handler with the logging system
 * @param handler The handler to register
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_handler_register(telemetry_log_handler handler);

/**
 * @brief Unregister a log handler from the logging system
 * @param handler The handler to unregister
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_handler_unregister(telemetry_log_handler handler);

/**
 * @brief Set a custom callback for a log handler
 * @param handler The handler to configure
 * @param callback The callback function
 * @param user_data User-defined data to pass to the callback
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_handler_set_callback(telemetry_log_handler handler, telemetry_log_handler_callback callback, void *user_data);

/**
 * @brief Configure file handler settings
 * @param handler The file handler to configure
 * @param file_path Path to the log file
 * @param rotation Rotation policy
 * @param max_size Maximum file size for rotation (bytes)
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_handler_configure_file(telemetry_log_handler handler, const char *file_path, telemetry_log_rotation_policy_id rotation, size_t max_size);

/**
 * @brief Create a new log formatter
 * @param format The format type
 * @return Pointer to the created formatter, or NULL on failure
 */
TELEMETRY_API telemetry_log_formatter telemetry_log_formatter_create(telemetry_log_format_id format);

/**
 * @brief Destroy a log formatter
 * @param formatter The formatter to destroy
 */
TELEMETRY_API void telemetry_log_formatter_destroy(telemetry_log_formatter formatter);

/**
 * @brief Set formatter options
 * @param formatter The formatter to configure
 * @param include_timestamp Include timestamp in output
 * @param include_level Include log level in output
 * @param include_category Include category in output
 * @param include_location Include file/line in output
 * @param include_thread_info Include thread info in output
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_formatter_configure(telemetry_log_formatter formatter, int include_timestamp, int include_level, int include_category, int include_location, int include_thread_info);

/**
 * @brief Set a custom formatter callback
 * @param formatter The formatter to configure
 * @param callback The callback function
 * @param user_data User-defined data to pass to the callback
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_formatter_set_callback(telemetry_log_formatter formatter, telemetry_log_formatter_callback callback, void *user_data);

/**
 * @brief Attach a formatter to a handler
 * @param handler The handler to attach the formatter to
 * @param formatter The formatter to attach
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_handler_set_formatter(telemetry_log_handler handler, telemetry_log_formatter formatter);

/**
 * @brief Create a new log filter
 * @return Pointer to the created filter, or NULL on failure
 */
TELEMETRY_API telemetry_log_filter telemetry_log_filter_create(void);

/**
 * @brief Destroy a log filter
 * @param filter The filter to destroy
 */
TELEMETRY_API void telemetry_log_filter_destroy(telemetry_log_filter filter);

/**
 * @brief Register a log filter with the logging system
 * @param filter The filter to register
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_filter_register(telemetry_log_filter filter);

/**
 * @brief Unregister a log filter from the logging system
 * @param filter The filter to unregister
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_filter_unregister(telemetry_log_filter filter);

/**
 * @brief Set a custom callback for a log filter
 * @param filter The filter to configure
 * @param callback The callback function
 * @param user_data User-defined data to pass to the callback
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_filter_set_callback(telemetry_log_filter filter, telemetry_log_filter_callback callback, void *user_data);

/**
 * @brief Set filter category pattern
 * @param filter The filter to configure
 * @param pattern Category pattern (supports wildcards)
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_filter_set_category(telemetry_log_filter filter, const char *pattern);

/**
 * @brief Set filter minimum level
 * @param filter The filter to configure
 * @param level Minimum log level
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_filter_set_level(telemetry_log_filter filter, telemetry_log_level_id level);

/**
 * @brief Log a message with full context
 * @param level Log severity level
 * @param category Log category/module
 * @param file Source file name
 * @param function Source function name
 * @param line Source line number
 * @param format Printf-style format string
 * @param ... Format arguments
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_write(telemetry_log_level_id level, const char *category, const char *file, const char *function, int line, const char *format, ...);

/**
 * @brief Log a message with a pre-formatted entry
 * @param entry The log entry to write
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_write_entry(telemetry_log_entry entry);

/**
 * @brief Enable or disable asynchronous logging
 * @param async Non-zero to enable, zero to disable
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_set_async(int async);

/**
 * @brief Flush all pending log entries
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_log_flush(void);

/**
 * @brief Get the string representation of a log level
 * @param level The log level
 * @return String representation of the level
 */
TELEMETRY_API const char *telemetry_log_level_to_string(telemetry_log_level_id level);

/**
 * @brief Parse a log level from a string
 * @param str String representation of the level
 * @return The log level, or TELEMETRY_LOG_LEVEL_INFO if invalid
 */
TELEMETRY_API telemetry_log_level_id telemetry_log_level_from_string(const char *str);

/* -- Convenience Macros -- */

#define TELEMETRY_LOG(level, category, ...) \
	telemetry_log_write(level, category, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define TELEMETRY_LOG_TRACE(category, ...) \
	TELEMETRY_LOG(TELEMETRY_LOG_LEVEL_TRACE, category, __VA_ARGS__)

#define TELEMETRY_LOG_DEBUG(category, ...) \
	TELEMETRY_LOG(TELEMETRY_LOG_LEVEL_DEBUG, category, __VA_ARGS__)

#define TELEMETRY_LOG_INFO(category, ...) \
	TELEMETRY_LOG(TELEMETRY_LOG_LEVEL_INFO, category, __VA_ARGS__)

#define TELEMETRY_LOG_WARNING(category, ...) \
	TELEMETRY_LOG(TELEMETRY_LOG_LEVEL_WARNING, category, __VA_ARGS__)

#define TELEMETRY_LOG_ERROR(category, ...) \
	TELEMETRY_LOG(TELEMETRY_LOG_LEVEL_ERROR, category, __VA_ARGS__)

#define TELEMETRY_LOG_CRITICAL(category, ...) \
	TELEMETRY_LOG(TELEMETRY_LOG_LEVEL_CRITICAL, category, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* TELEMETRY_LOG_H */
