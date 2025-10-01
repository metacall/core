/*
 *	Telemetry Library by MetaCall Inc.
 *	Configuration management for telemetry system.
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

#ifndef TELEMETRY_CONFIG_H
#define TELEMETRY_CONFIG_H 1

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <telemetry/telemetry_api.h>
#include <telemetry/telemetry_log.h>
#include <telemetry/telemetry_metrics.h>

/* -- Definitions -- */

#define TELEMETRY_CONFIG_MAX_STRING_SIZE 512
#define TELEMETRY_CONFIG_MAX_HANDLERS 16
#define TELEMETRY_CONFIG_MAX_EXPORTERS 8

/* -- Type Definitions -- */

/**
 * @brief Configuration for a log handler
 */
struct telemetry_config_handler_type
{
	telemetry_log_handler_type_id type;
	telemetry_log_level_id min_level;
	telemetry_log_format_id format;
	char file_path[TELEMETRY_CONFIG_MAX_STRING_SIZE];
	telemetry_log_rotation_policy_id rotation;
	size_t max_file_size;
	int enabled;
};

/**
 * @brief Configuration for a metric exporter
 */
struct telemetry_config_exporter_type
{
	telemetry_metric_export_format_id format;
	char endpoint[TELEMETRY_CONFIG_MAX_STRING_SIZE];
	int push_interval;
	int enabled;
};

/**
 * @brief Global telemetry configuration
 */
struct telemetry_config_type
{
	/* Logging configuration */
	telemetry_log_level_id log_level;
	int async_logging;
	struct telemetry_config_handler_type handlers[TELEMETRY_CONFIG_MAX_HANDLERS];
	size_t handler_count;

	/* Metrics configuration */
	struct telemetry_config_exporter_type exporters[TELEMETRY_CONFIG_MAX_EXPORTERS];
	size_t exporter_count;
	int metrics_enabled;

	/* General configuration */
	int enabled;
	char config_file_path[TELEMETRY_CONFIG_MAX_STRING_SIZE];
};

typedef struct telemetry_config_type *telemetry_config;

/* -- Methods -- */

/**
 * @brief Create a new telemetry configuration with defaults
 * @return Pointer to the created configuration, or NULL on failure
 */
TELEMETRY_API telemetry_config telemetry_config_create(void);

/**
 * @brief Destroy a telemetry configuration
 * @param config The configuration to destroy
 */
TELEMETRY_API void telemetry_config_destroy(telemetry_config config);

/**
 * @brief Load configuration from a JSON file
 * @param file_path Path to the configuration file
 * @return Pointer to the loaded configuration, or NULL on failure
 */
TELEMETRY_API telemetry_config telemetry_config_load_from_file(const char *file_path);

/**
 * @brief Load configuration from a JSON string
 * @param json_string JSON configuration string
 * @return Pointer to the loaded configuration, or NULL on failure
 */
TELEMETRY_API telemetry_config telemetry_config_load_from_string(const char *json_string);

/**
 * @brief Save configuration to a JSON file
 * @param config The configuration to save
 * @param file_path Path to save the configuration
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_config_save_to_file(telemetry_config config, const char *file_path);

/**
 * @brief Convert configuration to JSON string
 * @param config The configuration to convert
 * @param buffer Output buffer
 * @param size Buffer size
 * @return Number of bytes written, or negative on error
 */
TELEMETRY_API int telemetry_config_to_string(telemetry_config config, char *buffer, size_t size);

/**
 * @brief Apply configuration to the telemetry system
 * @param config The configuration to apply
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_config_apply(telemetry_config config);

/**
 * @brief Add a handler configuration
 * @param config The configuration
 * @param type Handler type
 * @param min_level Minimum log level
 * @param format Log format
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_config_add_handler(telemetry_config config, telemetry_log_handler_type_id type, telemetry_log_level_id min_level, telemetry_log_format_id format);

/**
 * @brief Add an exporter configuration
 * @param config The configuration
 * @param format Export format
 * @param endpoint Export endpoint
 * @param push_interval Push interval in seconds
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_config_add_exporter(telemetry_config config, telemetry_metric_export_format_id format, const char *endpoint, int push_interval);

/**
 * @brief Set log level in configuration
 * @param config The configuration
 * @param level The log level
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_config_set_log_level(telemetry_config config, telemetry_log_level_id level);

/**
 * @brief Enable or disable async logging in configuration
 * @param config The configuration
 * @param async Non-zero to enable, zero to disable
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_config_set_async_logging(telemetry_config config, int async);

/**
 * @brief Enable or disable metrics in configuration
 * @param config The configuration
 * @param enabled Non-zero to enable, zero to disable
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_config_set_metrics_enabled(telemetry_config config, int enabled);

/**
 * @brief Enable or disable telemetry in configuration
 * @param config The configuration
 * @param enabled Non-zero to enable, zero to disable
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_config_set_enabled(telemetry_config config, int enabled);

/**
 * @brief Get the global telemetry configuration
 * @return Pointer to the global configuration
 */
TELEMETRY_API telemetry_config telemetry_config_get_global(void);

/**
 * @brief Set the global telemetry configuration
 * @param config The configuration to set as global
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_config_set_global(telemetry_config config);

#ifdef __cplusplus
}
#endif

#endif /* TELEMETRY_CONFIG_H */
