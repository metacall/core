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

#ifndef TELEMETRY_METRICS_H
#define TELEMETRY_METRICS_H 1

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <telemetry/telemetry_api.h>

#include <stddef.h>
#include <stdint.h>
#include <time.h>

/* -- Definitions -- */

#define TELEMETRY_METRICS_MAX_NAME_SIZE 256
#define TELEMETRY_METRICS_MAX_LABELS 16
#define TELEMETRY_METRICS_MAX_LABEL_SIZE 128
#define TELEMETRY_METRICS_MAX_METRICS 1024
#define TELEMETRY_METRICS_MAX_EXPORTERS 8
#define TELEMETRY_METRICS_HISTOGRAM_BUCKETS 20

/* -- Type Definitions -- */

/**
 * @brief Metric types
 */
typedef enum telemetry_metric_type_id
{
	TELEMETRY_METRIC_TYPE_COUNTER = 0,   /**< Monotonically increasing counter */
	TELEMETRY_METRIC_TYPE_GAUGE = 1,     /**< Value that can go up and down */
	TELEMETRY_METRIC_TYPE_HISTOGRAM = 2, /**< Statistical distribution */
	TELEMETRY_METRIC_TYPE_SUMMARY = 3    /**< Similar to histogram with quantiles */
} telemetry_metric_type_id;

/**
 * @brief Metric aggregation types
 */
typedef enum telemetry_metric_aggregation_id
{
	TELEMETRY_METRIC_AGGREGATION_SUM = 0,    /**< Sum of values */
	TELEMETRY_METRIC_AGGREGATION_AVG = 1,    /**< Average of values */
	TELEMETRY_METRIC_AGGREGATION_MIN = 2,    /**< Minimum value */
	TELEMETRY_METRIC_AGGREGATION_MAX = 3,    /**< Maximum value */
	TELEMETRY_METRIC_AGGREGATION_COUNT = 4,  /**< Count of values */
	TELEMETRY_METRIC_AGGREGATION_RATE = 5    /**< Rate of change */
} telemetry_metric_aggregation_id;

/**
 * @brief Export format types
 */
typedef enum telemetry_metric_export_format_id
{
	TELEMETRY_METRIC_EXPORT_PROMETHEUS = 0, /**< Prometheus text format */
	TELEMETRY_METRIC_EXPORT_JSON = 1,       /**< JSON format */
	TELEMETRY_METRIC_EXPORT_STATSD = 2,     /**< StatsD format */
	TELEMETRY_METRIC_EXPORT_INFLUXDB = 3,   /**< InfluxDB line protocol */
	TELEMETRY_METRIC_EXPORT_GRAPHITE = 4    /**< Graphite plaintext format */
} telemetry_metric_export_format_id;

/**
 * @brief Forward declarations
 */
typedef struct telemetry_metric_label_type *telemetry_metric_label;
typedef struct telemetry_metric_type *telemetry_metric;
typedef struct telemetry_metric_counter_type *telemetry_metric_counter;
typedef struct telemetry_metric_gauge_type *telemetry_metric_gauge;
typedef struct telemetry_metric_histogram_type *telemetry_metric_histogram;
typedef struct telemetry_metric_registry_type *telemetry_metric_registry;
typedef struct telemetry_metric_exporter_type *telemetry_metric_exporter;

/**
 * @brief Label for metric dimensions
 */
struct telemetry_metric_label_type
{
	char key[TELEMETRY_METRICS_MAX_LABEL_SIZE];   /**< Label key */
	char value[TELEMETRY_METRICS_MAX_LABEL_SIZE]; /**< Label value */
};

/**
 * @brief Histogram bucket
 */
struct telemetry_histogram_bucket_type
{
	double upper_bound; /**< Upper bound of the bucket */
	uint64_t count;     /**< Count of observations in bucket */
};

/**
 * @brief Counter metric
 */
struct telemetry_metric_counter_type
{
	uint64_t value;                  /**< Current counter value */
	time_t last_updated;             /**< Last update timestamp */
	uint64_t increment_count;        /**< Number of increments */
};

/**
 * @brief Gauge metric
 */
struct telemetry_metric_gauge_type
{
	double value;                    /**< Current gauge value */
	double min_value;                /**< Minimum observed value */
	double max_value;                /**< Maximum observed value */
	time_t last_updated;             /**< Last update timestamp */
};

/**
 * @brief Histogram metric
 */
struct telemetry_metric_histogram_type
{
	struct telemetry_histogram_bucket_type buckets[TELEMETRY_METRICS_HISTOGRAM_BUCKETS]; /**< Histogram buckets */
	size_t bucket_count;             /**< Number of buckets */
	uint64_t total_count;            /**< Total number of observations */
	double sum;                      /**< Sum of all observations */
	double min;                      /**< Minimum observation */
	double max;                      /**< Maximum observation */
};

/**
 * @brief Generic metric structure
 */
struct telemetry_metric_type
{
	char name[TELEMETRY_METRICS_MAX_NAME_SIZE];                     /**< Metric name */
	char description[TELEMETRY_METRICS_MAX_NAME_SIZE];              /**< Metric description */
	char unit[64];                                                  /**< Unit of measurement */
	telemetry_metric_type_id type;                                  /**< Metric type */
	struct telemetry_metric_label_type labels[TELEMETRY_METRICS_MAX_LABELS]; /**< Metric labels */
	size_t label_count;                                             /**< Number of labels */
	time_t created_at;                                              /**< Creation timestamp */
	int enabled;                                                    /**< Enabled flag */

	union
	{
		struct telemetry_metric_counter_type counter;     /**< Counter data */
		struct telemetry_metric_gauge_type gauge;         /**< Gauge data */
		struct telemetry_metric_histogram_type histogram; /**< Histogram data */
	} data;
};

/**
 * @brief Callback function for custom metric exporters
 * @param metric The metric to export
 * @param buffer Output buffer
 * @param size Buffer size
 * @param user_data User-defined data
 * @return Number of bytes written, or negative on error
 */
typedef int (*telemetry_metric_exporter_callback)(telemetry_metric metric, char *buffer, size_t size, void *user_data);

/**
 * @brief Metric exporter
 */
struct telemetry_metric_exporter_type
{
	telemetry_metric_export_format_id format;     /**< Export format */
	telemetry_metric_exporter_callback callback;  /**< Custom exporter callback */
	void *user_data;                              /**< User-defined data */
	char endpoint[256];                           /**< Export endpoint (URL, file path, etc.) */
	int push_interval;                            /**< Push interval in seconds */
	time_t last_export;                           /**< Last export timestamp */
	int enabled;                                  /**< Enabled flag */
};

/**
 * @brief Metric registry
 */
struct telemetry_metric_registry_type
{
	telemetry_metric metrics[TELEMETRY_METRICS_MAX_METRICS];       /**< Registered metrics */
	size_t metric_count;                                           /**< Number of metrics */
	telemetry_metric_exporter exporters[TELEMETRY_METRICS_MAX_EXPORTERS]; /**< Registered exporters */
	size_t exporter_count;                                         /**< Number of exporters */
	void *mutex;                                                   /**< Thread safety mutex */
};

/* -- Methods -- */

/**
 * @brief Initialize the telemetry metrics system
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_metrics_initialize(void);

/**
 * @brief Shutdown the telemetry metrics system
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_metrics_shutdown(void);

/**
 * @brief Get the global metrics registry
 * @return Pointer to the global registry
 */
TELEMETRY_API telemetry_metric_registry telemetry_metrics_get_registry(void);

/**
 * @brief Create a new counter metric
 * @param name Metric name
 * @param description Metric description
 * @param unit Unit of measurement
 * @return Pointer to the created metric, or NULL on failure
 */
TELEMETRY_API telemetry_metric telemetry_metric_counter_create(const char *name, const char *description, const char *unit);

/**
 * @brief Create a new gauge metric
 * @param name Metric name
 * @param description Metric description
 * @param unit Unit of measurement
 * @return Pointer to the created metric, or NULL on failure
 */
TELEMETRY_API telemetry_metric telemetry_metric_gauge_create(const char *name, const char *description, const char *unit);

/**
 * @brief Create a new histogram metric
 * @param name Metric name
 * @param description Metric description
 * @param unit Unit of measurement
 * @param buckets Array of bucket upper bounds
 * @param bucket_count Number of buckets
 * @return Pointer to the created metric, or NULL on failure
 */
TELEMETRY_API telemetry_metric telemetry_metric_histogram_create(const char *name, const char *description, const char *unit, const double *buckets, size_t bucket_count);

/**
 * @brief Register a metric with the global registry
 * @param metric The metric to register
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_metric_register(telemetry_metric metric);

/**
 * @brief Unregister a metric from the global registry
 * @param metric The metric to unregister
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_metric_unregister(telemetry_metric metric);

/**
 * @brief Find a metric by name
 * @param name The metric name
 * @return Pointer to the metric, or NULL if not found
 */
TELEMETRY_API telemetry_metric telemetry_metric_find(const char *name);

/**
 * @brief Destroy a metric
 * @param metric The metric to destroy
 */
TELEMETRY_API void telemetry_metric_destroy(telemetry_metric metric);

/**
 * @brief Add a label to a metric
 * @param metric The metric
 * @param key Label key
 * @param value Label value
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_metric_add_label(telemetry_metric metric, const char *key, const char *value);

/**
 * @brief Increment a counter metric
 * @param metric The counter metric
 * @param value Value to increment by (default 1)
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_metric_counter_increment(telemetry_metric metric, uint64_t value);

/**
 * @brief Set a gauge metric value
 * @param metric The gauge metric
 * @param value New value
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_metric_gauge_set(telemetry_metric metric, double value);

/**
 * @brief Increment a gauge metric
 * @param metric The gauge metric
 * @param value Value to increment by
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_metric_gauge_increment(telemetry_metric metric, double value);

/**
 * @brief Decrement a gauge metric
 * @param metric The gauge metric
 * @param value Value to decrement by
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_metric_gauge_decrement(telemetry_metric metric, double value);

/**
 * @brief Observe a value in a histogram
 * @param metric The histogram metric
 * @param value Observed value
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_metric_histogram_observe(telemetry_metric metric, double value);

/**
 * @brief Get counter value
 * @param metric The counter metric
 * @return Current counter value, or 0 on error
 */
TELEMETRY_API uint64_t telemetry_metric_counter_get(telemetry_metric metric);

/**
 * @brief Get gauge value
 * @param metric The gauge metric
 * @return Current gauge value, or 0.0 on error
 */
TELEMETRY_API double telemetry_metric_gauge_get(telemetry_metric metric);

/**
 * @brief Get histogram statistics
 * @param metric The histogram metric
 * @param count Output: total count
 * @param sum Output: sum of observations
 * @param min Output: minimum value
 * @param max Output: maximum value
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_metric_histogram_get_stats(telemetry_metric metric, uint64_t *count, double *sum, double *min, double *max);

/**
 * @brief Create a metric exporter
 * @param format Export format
 * @param endpoint Export endpoint
 * @param push_interval Push interval in seconds (0 for manual export)
 * @return Pointer to the created exporter, or NULL on failure
 */
TELEMETRY_API telemetry_metric_exporter telemetry_metric_exporter_create(telemetry_metric_export_format_id format, const char *endpoint, int push_interval);

/**
 * @brief Destroy a metric exporter
 * @param exporter The exporter to destroy
 */
TELEMETRY_API void telemetry_metric_exporter_destroy(telemetry_metric_exporter exporter);

/**
 * @brief Register a metric exporter
 * @param exporter The exporter to register
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_metric_exporter_register(telemetry_metric_exporter exporter);

/**
 * @brief Unregister a metric exporter
 * @param exporter The exporter to unregister
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_metric_exporter_unregister(telemetry_metric_exporter exporter);

/**
 * @brief Set custom exporter callback
 * @param exporter The exporter
 * @param callback Custom callback function
 * @param user_data User-defined data
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_metric_exporter_set_callback(telemetry_metric_exporter exporter, telemetry_metric_exporter_callback callback, void *user_data);

/**
 * @brief Export all metrics using all registered exporters
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_metrics_export_all(void);

/**
 * @brief Export metrics to a buffer using specified format
 * @param format Export format
 * @param buffer Output buffer
 * @param size Buffer size
 * @return Number of bytes written, or negative on error
 */
TELEMETRY_API int telemetry_metrics_export_to_buffer(telemetry_metric_export_format_id format, char *buffer, size_t size);

/**
 * @brief Export a single metric to a buffer
 * @param metric The metric to export
 * @param format Export format
 * @param buffer Output buffer
 * @param size Buffer size
 * @return Number of bytes written, or negative on error
 */
TELEMETRY_API int telemetry_metric_export(telemetry_metric metric, telemetry_metric_export_format_id format, char *buffer, size_t size);

/**
 * @brief Reset a counter metric to zero
 * @param metric The counter metric
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_metric_counter_reset(telemetry_metric metric);

/**
 * @brief Reset a histogram metric
 * @param metric The histogram metric
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_metric_histogram_reset(telemetry_metric metric);

/**
 * @brief Reset all metrics in the registry
 * @return 0 on success, non-zero on failure
 */
TELEMETRY_API int telemetry_metrics_reset_all(void);

/**
 * @brief Get the number of registered metrics
 * @return Number of metrics
 */
TELEMETRY_API size_t telemetry_metrics_get_count(void);

/**
 * @brief Calculate aggregation over a metric (for gauges and histograms)
 * @param metric The metric
 * @param aggregation Aggregation type
 * @return Aggregated value
 */
TELEMETRY_API double telemetry_metric_aggregate(telemetry_metric metric, telemetry_metric_aggregation_id aggregation);

/* -- Convenience Macros -- */

#define TELEMETRY_COUNTER_INC(name) \
	do { \
		telemetry_metric m = telemetry_metric_find(name); \
		if (m != NULL) telemetry_metric_counter_increment(m, 1); \
	} while(0)

#define TELEMETRY_COUNTER_ADD(name, value) \
	do { \
		telemetry_metric m = telemetry_metric_find(name); \
		if (m != NULL) telemetry_metric_counter_increment(m, value); \
	} while(0)

#define TELEMETRY_GAUGE_SET(name, value) \
	do { \
		telemetry_metric m = telemetry_metric_find(name); \
		if (m != NULL) telemetry_metric_gauge_set(m, value); \
	} while(0)

#define TELEMETRY_GAUGE_INC(name, value) \
	do { \
		telemetry_metric m = telemetry_metric_find(name); \
		if (m != NULL) telemetry_metric_gauge_increment(m, value); \
	} while(0)

#define TELEMETRY_GAUGE_DEC(name, value) \
	do { \
		telemetry_metric m = telemetry_metric_find(name); \
		if (m != NULL) telemetry_metric_gauge_decrement(m, value); \
	} while(0)

#define TELEMETRY_HISTOGRAM_OBSERVE(name, value) \
	do { \
		telemetry_metric m = telemetry_metric_find(name); \
		if (m != NULL) telemetry_metric_histogram_observe(m, value); \
	} while(0)

#ifdef __cplusplus
}
#endif

#endif /* TELEMETRY_METRICS_H */
