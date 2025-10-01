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

/* -- Headers -- */

#include <telemetry/telemetry_metrics.h>

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* -- Private Data -- */

static struct telemetry_metric_registry_type global_registry = {
	.metric_count = 0,
	.exporter_count = 0,
	.mutex = NULL
};

static int telemetry_metrics_initialized = 0;

/* -- Private Forward Declarations -- */

static int telemetry_metric_export_prometheus(telemetry_metric metric, char *buffer, size_t size);
static int telemetry_metric_export_json(telemetry_metric metric, char *buffer, size_t size);
static int telemetry_metric_export_statsd(telemetry_metric metric, char *buffer, size_t size);
static int telemetry_metric_export_influxdb(telemetry_metric metric, char *buffer, size_t size);
static int telemetry_metric_export_graphite(telemetry_metric metric, char *buffer, size_t size);
static void telemetry_metric_escape_label_value(const char *input, char *output, size_t output_size);
static int telemetry_metric_labels_to_string(telemetry_metric metric, char *buffer, size_t size, const char *format);

/* -- Methods -- */

int telemetry_metrics_initialize(void)
{
	if (telemetry_metrics_initialized)
	{
		return 0;
	}

	/* Initialize the global registry */
	memset(&global_registry, 0, sizeof(struct telemetry_metric_registry_type));

	/* TODO: Initialize mutex for thread safety */

	telemetry_metrics_initialized = 1;

	return 0;
}

int telemetry_metrics_shutdown(void)
{
	size_t i;

	if (!telemetry_metrics_initialized)
	{
		return 0;
	}

	/* Destroy all metrics */
	for (i = 0; i < global_registry.metric_count; ++i)
	{
		if (global_registry.metrics[i] != NULL)
		{
			telemetry_metric_destroy(global_registry.metrics[i]);
			global_registry.metrics[i] = NULL;
		}
	}

	/* Destroy all exporters */
	for (i = 0; i < global_registry.exporter_count; ++i)
	{
		if (global_registry.exporters[i] != NULL)
		{
			telemetry_metric_exporter_destroy(global_registry.exporters[i]);
			global_registry.exporters[i] = NULL;
		}
	}

	/* TODO: Destroy mutex */

	telemetry_metrics_initialized = 0;

	return 0;
}

telemetry_metric_registry telemetry_metrics_get_registry(void)
{
	return &global_registry;
}

telemetry_metric telemetry_metric_counter_create(const char *name, const char *description, const char *unit)
{
	telemetry_metric metric;

	if (name == NULL)
	{
		return NULL;
	}

	metric = (telemetry_metric)malloc(sizeof(struct telemetry_metric_type));
	if (metric == NULL)
	{
		return NULL;
	}

	memset(metric, 0, sizeof(struct telemetry_metric_type));

	strncpy(metric->name, name, TELEMETRY_METRICS_MAX_NAME_SIZE - 1);
	metric->name[TELEMETRY_METRICS_MAX_NAME_SIZE - 1] = '\0';

	if (description != NULL)
	{
		strncpy(metric->description, description, TELEMETRY_METRICS_MAX_NAME_SIZE - 1);
		metric->description[TELEMETRY_METRICS_MAX_NAME_SIZE - 1] = '\0';
	}

	if (unit != NULL)
	{
		strncpy(metric->unit, unit, sizeof(metric->unit) - 1);
		metric->unit[sizeof(metric->unit) - 1] = '\0';
	}

	metric->type = TELEMETRY_METRIC_TYPE_COUNTER;
	metric->created_at = time(NULL);
	metric->enabled = 1;
	metric->label_count = 0;

	metric->data.counter.value = 0;
	metric->data.counter.last_updated = time(NULL);
	metric->data.counter.increment_count = 0;

	return metric;
}

telemetry_metric telemetry_metric_gauge_create(const char *name, const char *description, const char *unit)
{
	telemetry_metric metric;

	if (name == NULL)
	{
		return NULL;
	}

	metric = (telemetry_metric)malloc(sizeof(struct telemetry_metric_type));
	if (metric == NULL)
	{
		return NULL;
	}

	memset(metric, 0, sizeof(struct telemetry_metric_type));

	strncpy(metric->name, name, TELEMETRY_METRICS_MAX_NAME_SIZE - 1);
	metric->name[TELEMETRY_METRICS_MAX_NAME_SIZE - 1] = '\0';

	if (description != NULL)
	{
		strncpy(metric->description, description, TELEMETRY_METRICS_MAX_NAME_SIZE - 1);
		metric->description[TELEMETRY_METRICS_MAX_NAME_SIZE - 1] = '\0';
	}

	if (unit != NULL)
	{
		strncpy(metric->unit, unit, sizeof(metric->unit) - 1);
		metric->unit[sizeof(metric->unit) - 1] = '\0';
	}

	metric->type = TELEMETRY_METRIC_TYPE_GAUGE;
	metric->created_at = time(NULL);
	metric->enabled = 1;
	metric->label_count = 0;

	metric->data.gauge.value = 0.0;
	metric->data.gauge.min_value = DBL_MAX;
	metric->data.gauge.max_value = -DBL_MAX;
	metric->data.gauge.last_updated = time(NULL);

	return metric;
}

telemetry_metric telemetry_metric_histogram_create(const char *name, const char *description, const char *unit, const double *buckets, size_t bucket_count)
{
	telemetry_metric metric;
	size_t i;

	if (name == NULL || buckets == NULL || bucket_count == 0)
	{
		return NULL;
	}

	if (bucket_count > TELEMETRY_METRICS_HISTOGRAM_BUCKETS)
	{
		return NULL;
	}

	metric = (telemetry_metric)malloc(sizeof(struct telemetry_metric_type));
	if (metric == NULL)
	{
		return NULL;
	}

	memset(metric, 0, sizeof(struct telemetry_metric_type));

	strncpy(metric->name, name, TELEMETRY_METRICS_MAX_NAME_SIZE - 1);
	metric->name[TELEMETRY_METRICS_MAX_NAME_SIZE - 1] = '\0';

	if (description != NULL)
	{
		strncpy(metric->description, description, TELEMETRY_METRICS_MAX_NAME_SIZE - 1);
		metric->description[TELEMETRY_METRICS_MAX_NAME_SIZE - 1] = '\0';
	}

	if (unit != NULL)
	{
		strncpy(metric->unit, unit, sizeof(metric->unit) - 1);
		metric->unit[sizeof(metric->unit) - 1] = '\0';
	}

	metric->type = TELEMETRY_METRIC_TYPE_HISTOGRAM;
	metric->created_at = time(NULL);
	metric->enabled = 1;
	metric->label_count = 0;

	metric->data.histogram.bucket_count = bucket_count;
	metric->data.histogram.total_count = 0;
	metric->data.histogram.sum = 0.0;
	metric->data.histogram.min = DBL_MAX;
	metric->data.histogram.max = -DBL_MAX;

	for (i = 0; i < bucket_count; ++i)
	{
		metric->data.histogram.buckets[i].upper_bound = buckets[i];
		metric->data.histogram.buckets[i].count = 0;
	}

	return metric;
}

int telemetry_metric_register(telemetry_metric metric)
{
	if (metric == NULL)
	{
		return 1;
	}

	if (!telemetry_metrics_initialized)
	{
		telemetry_metrics_initialize();
	}

	if (global_registry.metric_count >= TELEMETRY_METRICS_MAX_METRICS)
	{
		return 1;
	}

	/* Check for duplicate names */
	if (telemetry_metric_find(metric->name) != NULL)
	{
		return 1;
	}

	global_registry.metrics[global_registry.metric_count++] = metric;

	return 0;
}

int telemetry_metric_unregister(telemetry_metric metric)
{
	size_t i, j;

	if (metric == NULL)
	{
		return 1;
	}

	for (i = 0; i < global_registry.metric_count; ++i)
	{
		if (global_registry.metrics[i] == metric)
		{
			/* Shift remaining metrics */
			for (j = i; j < global_registry.metric_count - 1; ++j)
			{
				global_registry.metrics[j] = global_registry.metrics[j + 1];
			}

			global_registry.metric_count--;
			return 0;
		}
	}

	return 1;
}

telemetry_metric telemetry_metric_find(const char *name)
{
	size_t i;

	if (name == NULL)
	{
		return NULL;
	}

	for (i = 0; i < global_registry.metric_count; ++i)
	{
		if (global_registry.metrics[i] != NULL)
		{
			if (strcmp(global_registry.metrics[i]->name, name) == 0)
			{
				return global_registry.metrics[i];
			}
		}
	}

	return NULL;
}

void telemetry_metric_destroy(telemetry_metric metric)
{
	if (metric == NULL)
	{
		return;
	}

	free(metric);
}

int telemetry_metric_add_label(telemetry_metric metric, const char *key, const char *value)
{
	if (metric == NULL || key == NULL || value == NULL)
	{
		return 1;
	}

	if (metric->label_count >= TELEMETRY_METRICS_MAX_LABELS)
	{
		return 1;
	}

	strncpy(metric->labels[metric->label_count].key, key, TELEMETRY_METRICS_MAX_LABEL_SIZE - 1);
	metric->labels[metric->label_count].key[TELEMETRY_METRICS_MAX_LABEL_SIZE - 1] = '\0';

	strncpy(metric->labels[metric->label_count].value, value, TELEMETRY_METRICS_MAX_LABEL_SIZE - 1);
	metric->labels[metric->label_count].value[TELEMETRY_METRICS_MAX_LABEL_SIZE - 1] = '\0';

	metric->label_count++;

	return 0;
}

int telemetry_metric_counter_increment(telemetry_metric metric, uint64_t value)
{
	if (metric == NULL || metric->type != TELEMETRY_METRIC_TYPE_COUNTER)
	{
		return 1;
	}

	if (!metric->enabled)
	{
		return 0;
	}

	metric->data.counter.value += value;
	metric->data.counter.increment_count++;
	metric->data.counter.last_updated = time(NULL);

	return 0;
}

int telemetry_metric_gauge_set(telemetry_metric metric, double value)
{
	if (metric == NULL || metric->type != TELEMETRY_METRIC_TYPE_GAUGE)
	{
		return 1;
	}

	if (!metric->enabled)
	{
		return 0;
	}

	metric->data.gauge.value = value;
	metric->data.gauge.last_updated = time(NULL);

	if (value < metric->data.gauge.min_value)
	{
		metric->data.gauge.min_value = value;
	}

	if (value > metric->data.gauge.max_value)
	{
		metric->data.gauge.max_value = value;
	}

	return 0;
}

int telemetry_metric_gauge_increment(telemetry_metric metric, double value)
{
	if (metric == NULL || metric->type != TELEMETRY_METRIC_TYPE_GAUGE)
	{
		return 1;
	}

	return telemetry_metric_gauge_set(metric, metric->data.gauge.value + value);
}

int telemetry_metric_gauge_decrement(telemetry_metric metric, double value)
{
	if (metric == NULL || metric->type != TELEMETRY_METRIC_TYPE_GAUGE)
	{
		return 1;
	}

	return telemetry_metric_gauge_set(metric, metric->data.gauge.value - value);
}

int telemetry_metric_histogram_observe(telemetry_metric metric, double value)
{
	size_t i;

	if (metric == NULL || metric->type != TELEMETRY_METRIC_TYPE_HISTOGRAM)
	{
		return 1;
	}

	if (!metric->enabled)
	{
		return 0;
	}

	/* Update sum and count */
	metric->data.histogram.sum += value;
	metric->data.histogram.total_count++;

	/* Update min and max */
	if (value < metric->data.histogram.min)
	{
		metric->data.histogram.min = value;
	}

	if (value > metric->data.histogram.max)
	{
		metric->data.histogram.max = value;
	}

	/* Update buckets */
	for (i = 0; i < metric->data.histogram.bucket_count; ++i)
	{
		if (value <= metric->data.histogram.buckets[i].upper_bound)
		{
			metric->data.histogram.buckets[i].count++;
			break;
		}
	}

	return 0;
}

uint64_t telemetry_metric_counter_get(telemetry_metric metric)
{
	if (metric == NULL || metric->type != TELEMETRY_METRIC_TYPE_COUNTER)
	{
		return 0;
	}

	return metric->data.counter.value;
}

double telemetry_metric_gauge_get(telemetry_metric metric)
{
	if (metric == NULL || metric->type != TELEMETRY_METRIC_TYPE_GAUGE)
	{
		return 0.0;
	}

	return metric->data.gauge.value;
}

int telemetry_metric_histogram_get_stats(telemetry_metric metric, uint64_t *count, double *sum, double *min, double *max)
{
	if (metric == NULL || metric->type != TELEMETRY_METRIC_TYPE_HISTOGRAM)
	{
		return 1;
	}

	if (count != NULL)
	{
		*count = metric->data.histogram.total_count;
	}

	if (sum != NULL)
	{
		*sum = metric->data.histogram.sum;
	}

	if (min != NULL)
	{
		*min = metric->data.histogram.min;
	}

	if (max != NULL)
	{
		*max = metric->data.histogram.max;
	}

	return 0;
}

telemetry_metric_exporter telemetry_metric_exporter_create(telemetry_metric_export_format_id format, const char *endpoint, int push_interval)
{
	telemetry_metric_exporter exporter;

	exporter = (telemetry_metric_exporter)malloc(sizeof(struct telemetry_metric_exporter_type));
	if (exporter == NULL)
	{
		return NULL;
	}

	memset(exporter, 0, sizeof(struct telemetry_metric_exporter_type));

	exporter->format = format;
	exporter->callback = NULL;
	exporter->user_data = NULL;
	exporter->push_interval = push_interval;
	exporter->last_export = time(NULL);
	exporter->enabled = 1;

	if (endpoint != NULL)
	{
		strncpy(exporter->endpoint, endpoint, sizeof(exporter->endpoint) - 1);
		exporter->endpoint[sizeof(exporter->endpoint) - 1] = '\0';
	}

	return exporter;
}

void telemetry_metric_exporter_destroy(telemetry_metric_exporter exporter)
{
	if (exporter == NULL)
	{
		return;
	}

	free(exporter);
}

int telemetry_metric_exporter_register(telemetry_metric_exporter exporter)
{
	if (exporter == NULL)
	{
		return 1;
	}

	if (global_registry.exporter_count >= TELEMETRY_METRICS_MAX_EXPORTERS)
	{
		return 1;
	}

	global_registry.exporters[global_registry.exporter_count++] = exporter;

	return 0;
}

int telemetry_metric_exporter_unregister(telemetry_metric_exporter exporter)
{
	size_t i, j;

	if (exporter == NULL)
	{
		return 1;
	}

	for (i = 0; i < global_registry.exporter_count; ++i)
	{
		if (global_registry.exporters[i] == exporter)
		{
			/* Shift remaining exporters */
			for (j = i; j < global_registry.exporter_count - 1; ++j)
			{
				global_registry.exporters[j] = global_registry.exporters[j + 1];
			}

			global_registry.exporter_count--;
			return 0;
		}
	}

	return 1;
}

int telemetry_metric_exporter_set_callback(telemetry_metric_exporter exporter, telemetry_metric_exporter_callback callback, void *user_data)
{
	if (exporter == NULL || callback == NULL)
	{
		return 1;
	}

	exporter->callback = callback;
	exporter->user_data = user_data;

	return 0;
}

int telemetry_metrics_export_all(void)
{
	size_t i;
	time_t now = time(NULL);
	int result = 0;

	/* Process each exporter */
	for (i = 0; i < global_registry.exporter_count; ++i)
	{
		telemetry_metric_exporter exporter = global_registry.exporters[i];

		if (exporter == NULL || !exporter->enabled)
		{
			continue;
		}

		/* Check if it's time to export */
		if (exporter->push_interval > 0)
		{
			if (difftime(now, exporter->last_export) < exporter->push_interval)
			{
				continue;
			}
		}

		/* Export metrics */
		char buffer[65536];
		int written = telemetry_metrics_export_to_buffer(exporter->format, buffer, sizeof(buffer));

		if (written > 0)
		{
			/* TODO: Send buffer to endpoint (file, network, etc.) */
			if (exporter->endpoint[0] != '\0')
			{
				FILE *fp = fopen(exporter->endpoint, "a");
				if (fp != NULL)
				{
					fprintf(fp, "%s", buffer);
					fclose(fp);
				}
			}

			exporter->last_export = now;
		}
		else
		{
			result = 1;
		}
	}

	return result;
}

int telemetry_metrics_export_to_buffer(telemetry_metric_export_format_id format, char *buffer, size_t size)
{
	size_t i;
	int total_written = 0;
	char metric_buffer[4096];

	if (buffer == NULL || size == 0)
	{
		return -1;
	}

	buffer[0] = '\0';

	/* Export each metric */
	for (i = 0; i < global_registry.metric_count; ++i)
	{
		telemetry_metric metric = global_registry.metrics[i];

		if (metric == NULL || !metric->enabled)
		{
			continue;
		}

		int written = telemetry_metric_export(metric, format, metric_buffer, sizeof(metric_buffer));

		if (written > 0 && (size_t)(total_written + written) < size)
		{
			strcat(buffer, metric_buffer);
			total_written += written;
		}
	}

	return total_written;
}

int telemetry_metric_export(telemetry_metric metric, telemetry_metric_export_format_id format, char *buffer, size_t size)
{
	if (metric == NULL || buffer == NULL || size == 0)
	{
		return -1;
	}

	switch (format)
	{
		case TELEMETRY_METRIC_EXPORT_PROMETHEUS:
			return telemetry_metric_export_prometheus(metric, buffer, size);

		case TELEMETRY_METRIC_EXPORT_JSON:
			return telemetry_metric_export_json(metric, buffer, size);

		case TELEMETRY_METRIC_EXPORT_STATSD:
			return telemetry_metric_export_statsd(metric, buffer, size);

		case TELEMETRY_METRIC_EXPORT_INFLUXDB:
			return telemetry_metric_export_influxdb(metric, buffer, size);

		case TELEMETRY_METRIC_EXPORT_GRAPHITE:
			return telemetry_metric_export_graphite(metric, buffer, size);

		default:
			return -1;
	}
}

int telemetry_metric_counter_reset(telemetry_metric metric)
{
	if (metric == NULL || metric->type != TELEMETRY_METRIC_TYPE_COUNTER)
	{
		return 1;
	}

	metric->data.counter.value = 0;
	metric->data.counter.increment_count = 0;
	metric->data.counter.last_updated = time(NULL);

	return 0;
}

int telemetry_metric_histogram_reset(telemetry_metric metric)
{
	size_t i;

	if (metric == NULL || metric->type != TELEMETRY_METRIC_TYPE_HISTOGRAM)
	{
		return 1;
	}

	metric->data.histogram.total_count = 0;
	metric->data.histogram.sum = 0.0;
	metric->data.histogram.min = DBL_MAX;
	metric->data.histogram.max = -DBL_MAX;

	for (i = 0; i < metric->data.histogram.bucket_count; ++i)
	{
		metric->data.histogram.buckets[i].count = 0;
	}

	return 0;
}

int telemetry_metrics_reset_all(void)
{
	size_t i;

	for (i = 0; i < global_registry.metric_count; ++i)
	{
		telemetry_metric metric = global_registry.metrics[i];

		if (metric == NULL)
		{
			continue;
		}

		switch (metric->type)
		{
			case TELEMETRY_METRIC_TYPE_COUNTER:
				telemetry_metric_counter_reset(metric);
				break;

			case TELEMETRY_METRIC_TYPE_HISTOGRAM:
				telemetry_metric_histogram_reset(metric);
				break;

			case TELEMETRY_METRIC_TYPE_GAUGE:
				/* Gauges typically aren't reset */
				break;

			default:
				break;
		}
	}

	return 0;
}

size_t telemetry_metrics_get_count(void)
{
	return global_registry.metric_count;
}

double telemetry_metric_aggregate(telemetry_metric metric, telemetry_metric_aggregation_id aggregation)
{
	if (metric == NULL)
	{
		return 0.0;
	}

	switch (metric->type)
	{
		case TELEMETRY_METRIC_TYPE_COUNTER:
			return (double)metric->data.counter.value;

		case TELEMETRY_METRIC_TYPE_GAUGE:
			switch (aggregation)
			{
				case TELEMETRY_METRIC_AGGREGATION_SUM:
				case TELEMETRY_METRIC_AGGREGATION_AVG:
					return metric->data.gauge.value;

				case TELEMETRY_METRIC_AGGREGATION_MIN:
					return metric->data.gauge.min_value;

				case TELEMETRY_METRIC_AGGREGATION_MAX:
					return metric->data.gauge.max_value;

				default:
					return metric->data.gauge.value;
			}

		case TELEMETRY_METRIC_TYPE_HISTOGRAM:
			switch (aggregation)
			{
				case TELEMETRY_METRIC_AGGREGATION_SUM:
					return metric->data.histogram.sum;

				case TELEMETRY_METRIC_AGGREGATION_AVG:
					if (metric->data.histogram.total_count > 0)
					{
						return metric->data.histogram.sum / metric->data.histogram.total_count;
					}
					return 0.0;

				case TELEMETRY_METRIC_AGGREGATION_MIN:
					return metric->data.histogram.min;

				case TELEMETRY_METRIC_AGGREGATION_MAX:
					return metric->data.histogram.max;

				case TELEMETRY_METRIC_AGGREGATION_COUNT:
					return (double)metric->data.histogram.total_count;

				default:
					return 0.0;
			}

		default:
			return 0.0;
	}
}

/* -- Private Methods -- */

static int telemetry_metric_export_prometheus(telemetry_metric metric, char *buffer, size_t size)
{
	int written = 0;
	int n;
	char labels_buf[512];

	if (metric == NULL || buffer == NULL || size == 0)
	{
		return -1;
	}

	buffer[0] = '\0';

	/* Format labels */
	telemetry_metric_labels_to_string(metric, labels_buf, sizeof(labels_buf), "prometheus");

	/* Add HELP and TYPE lines */
	if (metric->description[0] != '\0')
	{
		n = snprintf(buffer + written, size - written, "# HELP %s %s\n", metric->name, metric->description);
		if (n > 0)
			written += n;
	}

	switch (metric->type)
	{
		case TELEMETRY_METRIC_TYPE_COUNTER:
			n = snprintf(buffer + written, size - written, "# TYPE %s counter\n", metric->name);
			if (n > 0)
				written += n;
			n = snprintf(buffer + written, size - written, "%s%s %llu\n", metric->name, labels_buf, (unsigned long long)metric->data.counter.value);
			if (n > 0)
				written += n;
			break;

		case TELEMETRY_METRIC_TYPE_GAUGE:
			n = snprintf(buffer + written, size - written, "# TYPE %s gauge\n", metric->name);
			if (n > 0)
				written += n;
			n = snprintf(buffer + written, size - written, "%s%s %.6f\n", metric->name, labels_buf, metric->data.gauge.value);
			if (n > 0)
				written += n;
			break;

		case TELEMETRY_METRIC_TYPE_HISTOGRAM:
		{
			size_t i;
			n = snprintf(buffer + written, size - written, "# TYPE %s histogram\n", metric->name);
			if (n > 0)
				written += n;

			for (i = 0; i < metric->data.histogram.bucket_count; ++i)
			{
				n = snprintf(buffer + written, size - written, "%s_bucket{le=\"%.2f\"%s} %llu\n",
					metric->name,
					metric->data.histogram.buckets[i].upper_bound,
					labels_buf + 1, /* Skip opening brace */
					(unsigned long long)metric->data.histogram.buckets[i].count);
				if (n > 0)
					written += n;
			}

			n = snprintf(buffer + written, size - written, "%s_sum%s %.6f\n", metric->name, labels_buf, metric->data.histogram.sum);
			if (n > 0)
				written += n;
			n = snprintf(buffer + written, size - written, "%s_count%s %llu\n", metric->name, labels_buf, (unsigned long long)metric->data.histogram.total_count);
			if (n > 0)
				written += n;
			break;
		}

		default:
			break;
	}

	return written;
}

static int telemetry_metric_export_json(telemetry_metric metric, char *buffer, size_t size)
{
	int written = 0;
	int n;

	if (metric == NULL || buffer == NULL || size == 0)
	{
		return -1;
	}

	n = snprintf(buffer + written, size - written, "{\"name\":\"%s\",\"type\":\"%s\"",
		metric->name,
		metric->type == TELEMETRY_METRIC_TYPE_COUNTER ? "counter" : (metric->type == TELEMETRY_METRIC_TYPE_GAUGE ? "gauge" : "histogram"));
	if (n > 0)
		written += n;

	if (metric->description[0] != '\0')
	{
		n = snprintf(buffer + written, size - written, ",\"description\":\"%s\"", metric->description);
		if (n > 0)
			written += n;
	}

	if (metric->unit[0] != '\0')
	{
		n = snprintf(buffer + written, size - written, ",\"unit\":\"%s\"", metric->unit);
		if (n > 0)
			written += n;
	}

	/* Add value */
	switch (metric->type)
	{
		case TELEMETRY_METRIC_TYPE_COUNTER:
			n = snprintf(buffer + written, size - written, ",\"value\":%llu", (unsigned long long)metric->data.counter.value);
			if (n > 0)
				written += n;
			break;

		case TELEMETRY_METRIC_TYPE_GAUGE:
			n = snprintf(buffer + written, size - written, ",\"value\":%.6f,\"min\":%.6f,\"max\":%.6f",
				metric->data.gauge.value,
				metric->data.gauge.min_value,
				metric->data.gauge.max_value);
			if (n > 0)
				written += n;
			break;

		case TELEMETRY_METRIC_TYPE_HISTOGRAM:
			n = snprintf(buffer + written, size - written, ",\"count\":%llu,\"sum\":%.6f,\"min\":%.6f,\"max\":%.6f",
				(unsigned long long)metric->data.histogram.total_count,
				metric->data.histogram.sum,
				metric->data.histogram.min,
				metric->data.histogram.max);
			if (n > 0)
				written += n;
			break;

		default:
			break;
	}

	/* Add labels if present */
	if (metric->label_count > 0)
	{
		size_t i;
		n = snprintf(buffer + written, size - written, ",\"labels\":{");
		if (n > 0)
			written += n;

		for (i = 0; i < metric->label_count; ++i)
		{
			n = snprintf(buffer + written, size - written, "%s\"%s\":\"%s\"",
				i > 0 ? "," : "",
				metric->labels[i].key,
				metric->labels[i].value);
			if (n > 0)
				written += n;
		}

		n = snprintf(buffer + written, size - written, "}");
		if (n > 0)
			written += n;
	}

	n = snprintf(buffer + written, size - written, "}\n");
	if (n > 0)
		written += n;

	return written;
}

static int telemetry_metric_export_statsd(telemetry_metric metric, char *buffer, size_t size)
{
	int written = 0;

	if (metric == NULL || buffer == NULL || size == 0)
	{
		return -1;
	}

	switch (metric->type)
	{
		case TELEMETRY_METRIC_TYPE_COUNTER:
			written = snprintf(buffer, size, "%s:%llu|c\n", metric->name, (unsigned long long)metric->data.counter.value);
			break;

		case TELEMETRY_METRIC_TYPE_GAUGE:
			written = snprintf(buffer, size, "%s:%.6f|g\n", metric->name, metric->data.gauge.value);
			break;

		case TELEMETRY_METRIC_TYPE_HISTOGRAM:
			written = snprintf(buffer, size, "%s:%.6f|h\n", metric->name, metric->data.histogram.sum);
			break;

		default:
			written = -1;
			break;
	}

	return written;
}

static int telemetry_metric_export_influxdb(telemetry_metric metric, char *buffer, size_t size)
{
	int written = 0;
	int n;
	char labels_buf[512];

	if (metric == NULL || buffer == NULL || size == 0)
	{
		return -1;
	}

	/* Format labels as tags */
	telemetry_metric_labels_to_string(metric, labels_buf, sizeof(labels_buf), "influxdb");

	/* Measurement name */
	n = snprintf(buffer + written, size - written, "%s%s ", metric->name, labels_buf);
	if (n > 0)
		written += n;

	/* Fields */
	switch (metric->type)
	{
		case TELEMETRY_METRIC_TYPE_COUNTER:
			n = snprintf(buffer + written, size - written, "value=%llui", (unsigned long long)metric->data.counter.value);
			if (n > 0)
				written += n;
			break;

		case TELEMETRY_METRIC_TYPE_GAUGE:
			n = snprintf(buffer + written, size - written, "value=%.6f,min=%.6f,max=%.6f",
				metric->data.gauge.value,
				metric->data.gauge.min_value,
				metric->data.gauge.max_value);
			if (n > 0)
				written += n;
			break;

		case TELEMETRY_METRIC_TYPE_HISTOGRAM:
			n = snprintf(buffer + written, size - written, "count=%llui,sum=%.6f,min=%.6f,max=%.6f",
				(unsigned long long)metric->data.histogram.total_count,
				metric->data.histogram.sum,
				metric->data.histogram.min,
				metric->data.histogram.max);
			if (n > 0)
				written += n;
			break;

		default:
			break;
	}

	n = snprintf(buffer + written, size - written, "\n");
	if (n > 0)
		written += n;

	return written;
}

static int telemetry_metric_export_graphite(telemetry_metric metric, char *buffer, size_t size)
{
	int written = 0;
	time_t now = time(NULL);

	if (metric == NULL || buffer == NULL || size == 0)
	{
		return -1;
	}

	switch (metric->type)
	{
		case TELEMETRY_METRIC_TYPE_COUNTER:
			written = snprintf(buffer, size, "%s %llu %ld\n",
				metric->name,
				(unsigned long long)metric->data.counter.value,
				(long)now);
			break;

		case TELEMETRY_METRIC_TYPE_GAUGE:
			written = snprintf(buffer, size, "%s %.6f %ld\n",
				metric->name,
				metric->data.gauge.value,
				(long)now);
			break;

		case TELEMETRY_METRIC_TYPE_HISTOGRAM:
		{
			int n;
			n = snprintf(buffer + written, size - written, "%s.count %llu %ld\n",
				metric->name,
				(unsigned long long)metric->data.histogram.total_count,
				(long)now);
			if (n > 0)
				written += n;

			n = snprintf(buffer + written, size - written, "%s.sum %.6f %ld\n",
				metric->name,
				metric->data.histogram.sum,
				(long)now);
			if (n > 0)
				written += n;
			break;
		}

		default:
			written = -1;
			break;
	}

	return written;
}

static void telemetry_metric_escape_label_value(const char *input, char *output, size_t output_size)
{
	size_t i, j;
	size_t input_len = strlen(input);

	if (input == NULL || output == NULL || output_size == 0)
	{
		return;
	}

	for (i = 0, j = 0; i < input_len && j < output_size - 1; ++i)
	{
		char c = input[i];

		/* Escape special characters */
		if (c == '"' || c == '\\' || c == '\n')
		{
			if (j < output_size - 2)
			{
				output[j++] = '\\';
				output[j++] = c == '\n' ? 'n' : c;
			}
		}
		else
		{
			output[j++] = c;
		}
	}

	output[j] = '\0';
}

static int telemetry_metric_labels_to_string(telemetry_metric metric, char *buffer, size_t size, const char *format)
{
	size_t i;
	int written = 0;
	int n;
	char escaped[TELEMETRY_METRICS_MAX_LABEL_SIZE * 2];

	if (metric == NULL || buffer == NULL || size == 0)
	{
		return -1;
	}

	buffer[0] = '\0';

	if (metric->label_count == 0)
	{
		return 0;
	}

	if (strcmp(format, "prometheus") == 0)
	{
		n = snprintf(buffer + written, size - written, "{");
		if (n > 0)
			written += n;

		for (i = 0; i < metric->label_count; ++i)
		{
			telemetry_metric_escape_label_value(metric->labels[i].value, escaped, sizeof(escaped));

			n = snprintf(buffer + written, size - written, "%s%s=\"%s\"",
				i > 0 ? "," : "",
				metric->labels[i].key,
				escaped);
			if (n > 0)
				written += n;
		}

		n = snprintf(buffer + written, size - written, "}");
		if (n > 0)
			written += n;
	}
	else if (strcmp(format, "influxdb") == 0)
	{
		for (i = 0; i < metric->label_count; ++i)
		{
			n = snprintf(buffer + written, size - written, ",%s=%s",
				metric->labels[i].key,
				metric->labels[i].value);
			if (n > 0)
				written += n;
		}
	}

	return written;
}
