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

import java.util.Arrays;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.atomic.AtomicReference;

/**
 * Histogram metric - statistical distribution
 */
public class Histogram extends Metric {
	private final double[] buckets;
	private final AtomicLong[] bucketCounts;
	private final AtomicLong totalCount;
	private final AtomicReference<Double> sum;
	private volatile double min;
	private volatile double max;

	private static final double[] DEFAULT_BUCKETS = {
		0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 5.0, 10.0
	};

	public Histogram(String name) {
		this(name, "", "", DEFAULT_BUCKETS);
	}

	public Histogram(String name, String description) {
		this(name, description, "", DEFAULT_BUCKETS);
	}

	public Histogram(String name, String description, String unit) {
		this(name, description, unit, DEFAULT_BUCKETS);
	}

	public Histogram(String name, String description, String unit, double[] buckets) {
		super(name, description, unit);
		this.buckets = Arrays.copyOf(buckets, buckets.length);
		this.bucketCounts = new AtomicLong[buckets.length];
		for (int i = 0; i < buckets.length; i++) {
			this.bucketCounts[i] = new AtomicLong(0);
		}
		this.totalCount = new AtomicLong(0);
		this.sum = new AtomicReference<>(0.0);
		this.min = Double.MAX_VALUE;
		this.max = -Double.MAX_VALUE;
	}

	/**
	 * Observe a value
	 * @param value The value to observe
	 */
	public void observe(double value) {
		// TODO: Call native C library function

		// Update sum and count
		double oldSum;
		do {
			oldSum = sum.get();
		} while (!sum.compareAndSet(oldSum, oldSum + value));
		totalCount.incrementAndGet();

		// Update min and max
		if (value < min) {
			min = value;
		}
		if (value > max) {
			max = value;
		}

		// Update buckets
		for (int i = 0; i < buckets.length; i++) {
			if (value <= buckets[i]) {
				bucketCounts[i].incrementAndGet();
			}
		}
	}

	/**
	 * Get histogram statistics
	 * @return Statistics object
	 */
	public HistogramStats getStats() {
		// TODO: Call native C library function
		return new HistogramStats(totalCount.get(), sum.get(), min, max);
	}

	/**
	 * Get the bucket configuration
	 * @return Array of bucket upper bounds
	 */
	public double[] getBuckets() {
		return Arrays.copyOf(buckets, buckets.length);
	}

	/**
	 * Get the count for a specific bucket
	 * @param index The bucket index
	 * @return The count for that bucket
	 */
	public long getBucketCount(int index) {
		if (index < 0 || index >= bucketCounts.length) {
			throw new IndexOutOfBoundsException("Bucket index out of range");
		}
		return bucketCounts[index].get();
	}

	/**
	 * Reset the histogram
	 */
	public void reset() {
		// TODO: Call native C library function
		totalCount.set(0);
		sum.set(0.0);
		min = Double.MAX_VALUE;
		max = -Double.MAX_VALUE;
		for (AtomicLong count : bucketCounts) {
			count.set(0);
		}
	}
}
