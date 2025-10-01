/*
 * Telemetry Library Java Bindings
 * A Java interface for the MetaCall telemetry and logging system.
 *
 * Copyright (C) 2025 MetaCall Inc., Dhiren Mhatre
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

package com.metacall.telemetry;

import java.util.HashMap;
import java.util.Map;
import java.util.Collection;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Registry for managing metrics
 */
public class MetricRegistry {
	private static boolean initialized = false;
	private static final Map<String, Metric> metrics = new HashMap<>();
	private static final ReentrantLock lock = new ReentrantLock();

	/**
	 * Initialize the metrics system
	 */
	public static void initialize() {
		if (!initialized) {
			lock.lock();
			try {
				if (!initialized) {
					// TODO: Call native C library telemetry_metrics_initialize()
					initialized = true;
				}
			} finally {
				lock.unlock();
			}
		}
	}

	/**
	 * Shutdown the metrics system
	 */
	public static void shutdown() {
		if (initialized) {
			lock.lock();
			try {
				if (initialized) {
					// TODO: Call native C library telemetry_metrics_shutdown()
					metrics.clear();
					initialized = false;
				}
			} finally {
				lock.unlock();
			}
		}
	}

	/**
	 * Register a metric
	 * @param metric The metric to register
	 */
	public static void register(Metric metric) {
		initialize();
		lock.lock();
		try {
			if (metrics.containsKey(metric.getName())) {
				throw new IllegalArgumentException("Metric '" + metric.getName() + "' already registered");
			}
			metrics.put(metric.getName(), metric);
			metric.register();
		} finally {
			lock.unlock();
		}
	}

	/**
	 * Unregister a metric
	 * @param metric The metric to unregister
	 */
	public static void unregister(Metric metric) {
		lock.lock();
		try {
			if (metrics.remove(metric.getName()) != null) {
				metric.unregister();
			}
		} finally {
			lock.unlock();
		}
	}

	/**
	 * Get a metric by name
	 * @param name The metric name
	 * @return The metric, or null if not found
	 */
	public static Metric get(String name) {
		lock.lock();
		try {
			return metrics.get(name);
		} finally {
			lock.unlock();
		}
	}

	/**
	 * Get all registered metrics
	 * @return Collection of all metrics
	 */
	public static Collection<Metric> getAll() {
		lock.lock();
		try {
			return new HashMap<>(metrics).values();
		} finally {
			lock.unlock();
		}
	}

	/**
	 * Export all metrics to string format
	 * @param format The export format
	 * @return Exported metrics as string
	 */
	public static String exportAll(MetricExportFormat format) {
		initialize();
		// TODO: Call native C library function

		// Fallback implementation for Prometheus format
		if (format == MetricExportFormat.PROMETHEUS) {
			StringBuilder sb = new StringBuilder();

			for (Metric metric : metrics.values()) {
				if (!metric.getDescription().isEmpty()) {
					sb.append("# HELP ").append(metric.getName()).append(" ")
					  .append(metric.getDescription()).append("\n");
				}

				if (metric instanceof Counter) {
					Counter counter = (Counter) metric;
					sb.append("# TYPE ").append(metric.getName()).append(" counter\n");
					sb.append(metric.getName()).append(metric.getLabelsString())
					  .append(" ").append(counter.get()).append("\n");
				} else if (metric instanceof Gauge) {
					Gauge gauge = (Gauge) metric;
					sb.append("# TYPE ").append(metric.getName()).append(" gauge\n");
					sb.append(metric.getName()).append(metric.getLabelsString())
					  .append(" ").append(gauge.get()).append("\n");
				} else if (metric instanceof Histogram) {
					Histogram histogram = (Histogram) metric;
					sb.append("# TYPE ").append(metric.getName()).append(" histogram\n");

					HistogramStats stats = histogram.getStats();
					sb.append(metric.getName()).append("_sum").append(metric.getLabelsString())
					  .append(" ").append(stats.getSum()).append("\n");
					sb.append(metric.getName()).append("_count").append(metric.getLabelsString())
					  .append(" ").append(stats.getCount()).append("\n");
				}

				sb.append("\n");
			}

			return sb.toString();
		}

		return "";
	}

	/**
	 * Reset all metrics
	 */
	public static void resetAll() {
		initialize();
		// TODO: Call native C library function

		lock.lock();
		try {
			for (Metric metric : metrics.values()) {
				if (metric instanceof Counter) {
					((Counter) metric).reset();
				} else if (metric instanceof Histogram) {
					((Histogram) metric).reset();
				}
			}
		} finally {
			lock.unlock();
		}
	}

	/**
	 * Get the number of registered metrics
	 * @return Number of metrics
	 */
	public static int getCount() {
		lock.lock();
		try {
			return metrics.size();
		} finally {
			lock.unlock();
		}
	}
}
