/*
 *	Telemetry Library by MetaCall Inc.
 *	Test suite for telemetry metrics system.
 *
 *	Copyright (C) 2025 MetaCall Inc., Dhiren Mhatre
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 */

#include <telemetry/telemetry_metrics.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Test initialization and shutdown */
void test_metrics_init_shutdown(void)
{
	printf("Testing metrics initialization and shutdown...\n");

	int result = telemetry_metrics_initialize();
	assert(result == 0);

	result = telemetry_metrics_shutdown();
	assert(result == 0);

	printf("✓ Metrics initialization and shutdown test passed\n");
}

/* Test counter creation and operations */
void test_counter_metrics(void)
{
	printf("Testing counter metrics...\n");

	telemetry_metrics_initialize();

	telemetry_metric counter = telemetry_metric_counter_create(
		"test_counter",
		"A test counter",
		"requests");
	assert(counter != NULL);

	int result = telemetry_metric_register(counter);
	assert(result == 0);

	/* Test increment */
	result = telemetry_metric_counter_increment(counter, 5);
	assert(result == 0);
	assert(telemetry_metric_counter_get(counter) == 5);

	result = telemetry_metric_counter_increment(counter, 3);
	assert(result == 0);
	assert(telemetry_metric_counter_get(counter) == 8);

	/* Test reset */
	result = telemetry_metric_counter_reset(counter);
	assert(result == 0);
	assert(telemetry_metric_counter_get(counter) == 0);

	telemetry_metric_unregister(counter);
	telemetry_metric_destroy(counter);

	telemetry_metrics_shutdown();

	printf("✓ Counter metrics test passed\n");
}

/* Test gauge creation and operations */
void test_gauge_metrics(void)
{
	printf("Testing gauge metrics...\n");

	telemetry_metrics_initialize();

	telemetry_metric gauge = telemetry_metric_gauge_create(
		"test_gauge",
		"A test gauge",
		"bytes");
	assert(gauge != NULL);

	int result = telemetry_metric_register(gauge);
	assert(result == 0);

	/* Test set */
	result = telemetry_metric_gauge_set(gauge, 42.5);
	assert(result == 0);
	assert(telemetry_metric_gauge_get(gauge) == 42.5);

	/* Test increment */
	result = telemetry_metric_gauge_increment(gauge, 7.5);
	assert(result == 0);
	assert(telemetry_metric_gauge_get(gauge) == 50.0);

	/* Test decrement */
	result = telemetry_metric_gauge_decrement(gauge, 10.0);
	assert(result == 0);
	assert(telemetry_metric_gauge_get(gauge) == 40.0);

	telemetry_metric_unregister(gauge);
	telemetry_metric_destroy(gauge);

	telemetry_metrics_shutdown();

	printf("✓ Gauge metrics test passed\n");
}

/* Test histogram creation and operations */
void test_histogram_metrics(void)
{
	printf("Testing histogram metrics...\n");

	telemetry_metrics_initialize();

	double buckets[] = { 0.1, 0.5, 1.0, 5.0, 10.0 };
	size_t bucket_count = sizeof(buckets) / sizeof(buckets[0]);

	telemetry_metric histogram = telemetry_metric_histogram_create(
		"test_histogram",
		"A test histogram",
		"seconds",
		buckets,
		bucket_count);
	assert(histogram != NULL);

	int result = telemetry_metric_register(histogram);
	assert(result == 0);

	/* Test observations */
	result = telemetry_metric_histogram_observe(histogram, 0.05);
	assert(result == 0);

	result = telemetry_metric_histogram_observe(histogram, 0.3);
	assert(result == 0);

	result = telemetry_metric_histogram_observe(histogram, 0.8);
	assert(result == 0);

	result = telemetry_metric_histogram_observe(histogram, 2.5);
	assert(result == 0);

	result = telemetry_metric_histogram_observe(histogram, 7.0);
	assert(result == 0);

	/* Test statistics */
	uint64_t count;
	double sum, min, max;
	result = telemetry_metric_histogram_get_stats(histogram, &count, &sum, &min, &max);
	assert(result == 0);
	assert(count == 5);
	assert(sum == (0.05 + 0.3 + 0.8 + 2.5 + 7.0));
	assert(min == 0.05);
	assert(max == 7.0);

	/* Test reset */
	result = telemetry_metric_histogram_reset(histogram);
	assert(result == 0);

	result = telemetry_metric_histogram_get_stats(histogram, &count, &sum, &min, &max);
	assert(result == 0);
	assert(count == 0);
	assert(sum == 0.0);

	telemetry_metric_unregister(histogram);
	telemetry_metric_destroy(histogram);

	telemetry_metrics_shutdown();

	printf("✓ Histogram metrics test passed\n");
}

/* Test metric labels */
void test_metric_labels(void)
{
	printf("Testing metric labels...\n");

	telemetry_metrics_initialize();

	telemetry_metric counter = telemetry_metric_counter_create(
		"http_requests_total",
		"Total HTTP requests",
		"requests");
	assert(counter != NULL);

	int result = telemetry_metric_add_label(counter, "method", "GET");
	assert(result == 0);

	result = telemetry_metric_add_label(counter, "status", "200");
	assert(result == 0);

	result = telemetry_metric_add_label(counter, "endpoint", "/api/users");
	assert(result == 0);

	result = telemetry_metric_register(counter);
	assert(result == 0);

	telemetry_metric_unregister(counter);
	telemetry_metric_destroy(counter);

	telemetry_metrics_shutdown();

	printf("✓ Metric labels test passed\n");
}

/* Test metric find */
void test_metric_find(void)
{
	printf("Testing metric find...\n");

	telemetry_metrics_initialize();

	telemetry_metric counter = telemetry_metric_counter_create(
		"findable_counter",
		"A counter that can be found",
		"");
	telemetry_metric_register(counter);

	telemetry_metric found = telemetry_metric_find("findable_counter");
	assert(found != NULL);
	assert(found == counter);

	telemetry_metric not_found = telemetry_metric_find("nonexistent_metric");
	assert(not_found == NULL);

	telemetry_metric_unregister(counter);
	telemetry_metric_destroy(counter);

	telemetry_metrics_shutdown();

	printf("✓ Metric find test passed\n");
}

/* Test metric exporters */
void test_metric_exporters(void)
{
	printf("Testing metric exporters...\n");

	telemetry_metrics_initialize();

	telemetry_metric_exporter prometheus_exporter = telemetry_metric_exporter_create(
		TELEMETRY_METRIC_EXPORT_PROMETHEUS,
		"/tmp/metrics.txt",
		60);
	assert(prometheus_exporter != NULL);

	int result = telemetry_metric_exporter_register(prometheus_exporter);
	assert(result == 0);

	result = telemetry_metric_exporter_unregister(prometheus_exporter);
	assert(result == 0);

	telemetry_metric_exporter_destroy(prometheus_exporter);

	telemetry_metrics_shutdown();

	printf("✓ Metric exporters test passed\n");
}

/* Test Prometheus export format */
void test_prometheus_export(void)
{
	printf("Testing Prometheus export format...\n");

	telemetry_metrics_initialize();

	/* Create and register a counter */
	telemetry_metric counter = telemetry_metric_counter_create(
		"test_requests_total",
		"Total test requests",
		"requests");
	telemetry_metric_add_label(counter, "method", "GET");
	telemetry_metric_counter_increment(counter, 42);
	telemetry_metric_register(counter);

	/* Create and register a gauge */
	telemetry_metric gauge = telemetry_metric_gauge_create(
		"test_memory_usage",
		"Test memory usage",
		"bytes");
	telemetry_metric_gauge_set(gauge, 1024.0);
	telemetry_metric_register(gauge);

	/* Export to Prometheus format */
	char buffer[4096];
	int written = telemetry_metrics_export_to_buffer(
		TELEMETRY_METRIC_EXPORT_PROMETHEUS,
		buffer,
		sizeof(buffer));

	assert(written > 0);
	printf("Prometheus export:\n%s\n", buffer);

	telemetry_metric_unregister(counter);
	telemetry_metric_unregister(gauge);
	telemetry_metric_destroy(counter);
	telemetry_metric_destroy(gauge);

	telemetry_metrics_shutdown();

	printf("✓ Prometheus export test passed\n");
}

/* Test JSON export format */
void test_json_export(void)
{
	printf("Testing JSON export format...\n");

	telemetry_metrics_initialize();

	telemetry_metric counter = telemetry_metric_counter_create(
		"api_calls",
		"API calls counter",
		"calls");
	telemetry_metric_counter_increment(counter, 100);
	telemetry_metric_register(counter);

	char buffer[4096];
	int written = telemetry_metrics_export_to_buffer(
		TELEMETRY_METRIC_EXPORT_JSON,
		buffer,
		sizeof(buffer));

	assert(written > 0);
	printf("JSON export:\n%s\n", buffer);

	telemetry_metric_unregister(counter);
	telemetry_metric_destroy(counter);

	telemetry_metrics_shutdown();

	printf("✓ JSON export test passed\n");
}

/* Test metric aggregation */
void test_metric_aggregation(void)
{
	printf("Testing metric aggregation...\n");

	telemetry_metrics_initialize();

	telemetry_metric gauge = telemetry_metric_gauge_create(
		"temperature",
		"Temperature gauge",
		"celsius");
	telemetry_metric_register(gauge);

	telemetry_metric_gauge_set(gauge, 20.0);
	telemetry_metric_gauge_set(gauge, 25.0);
	telemetry_metric_gauge_set(gauge, 15.0);

	double current = telemetry_metric_aggregate(gauge, TELEMETRY_METRIC_AGGREGATION_SUM);
	assert(current == 15.0); /* Last value */

	double min_val = telemetry_metric_aggregate(gauge, TELEMETRY_METRIC_AGGREGATION_MIN);
	assert(min_val == 15.0);

	double max_val = telemetry_metric_aggregate(gauge, TELEMETRY_METRIC_AGGREGATION_MAX);
	assert(max_val == 25.0);

	telemetry_metric_unregister(gauge);
	telemetry_metric_destroy(gauge);

	telemetry_metrics_shutdown();

	printf("✓ Metric aggregation test passed\n");
}

/* Test reset all metrics */
void test_reset_all_metrics(void)
{
	printf("Testing reset all metrics...\n");

	telemetry_metrics_initialize();

	telemetry_metric counter1 = telemetry_metric_counter_create("counter1", "", "");
	telemetry_metric counter2 = telemetry_metric_counter_create("counter2", "", "");

	telemetry_metric_counter_increment(counter1, 10);
	telemetry_metric_counter_increment(counter2, 20);

	telemetry_metric_register(counter1);
	telemetry_metric_register(counter2);

	assert(telemetry_metric_counter_get(counter1) == 10);
	assert(telemetry_metric_counter_get(counter2) == 20);

	telemetry_metrics_reset_all();

	assert(telemetry_metric_counter_get(counter1) == 0);
	assert(telemetry_metric_counter_get(counter2) == 0);

	telemetry_metric_unregister(counter1);
	telemetry_metric_unregister(counter2);
	telemetry_metric_destroy(counter1);
	telemetry_metric_destroy(counter2);

	telemetry_metrics_shutdown();

	printf("✓ Reset all metrics test passed\n");
}

/* Test metrics count */
void test_metrics_count(void)
{
	printf("Testing metrics count...\n");

	telemetry_metrics_initialize();

	size_t initial_count = telemetry_metrics_get_count();

	telemetry_metric m1 = telemetry_metric_counter_create("m1", "", "");
	telemetry_metric m2 = telemetry_metric_gauge_create("m2", "", "");
	double buckets[] = { 1.0, 5.0, 10.0 };
	telemetry_metric m3 = telemetry_metric_histogram_create("m3", "", "", buckets, 3);

	telemetry_metric_register(m1);
	telemetry_metric_register(m2);
	telemetry_metric_register(m3);

	assert(telemetry_metrics_get_count() == initial_count + 3);

	telemetry_metric_unregister(m1);
	assert(telemetry_metrics_get_count() == initial_count + 2);

	telemetry_metric_unregister(m2);
	telemetry_metric_unregister(m3);

	telemetry_metric_destroy(m1);
	telemetry_metric_destroy(m2);
	telemetry_metric_destroy(m3);

	telemetry_metrics_shutdown();

	printf("✓ Metrics count test passed\n");
}

/* Main test runner */
int main(void)
{
	printf("=== Telemetry Metrics System Tests ===\n\n");

	test_metrics_init_shutdown();
	test_counter_metrics();
	test_gauge_metrics();
	test_histogram_metrics();
	test_metric_labels();
	test_metric_find();
	test_metric_exporters();
	test_prometheus_export();
	test_json_export();
	test_metric_aggregation();
	test_reset_all_metrics();
	test_metrics_count();

	printf("\n=== All Tests Passed ===\n");

	return 0;
}
