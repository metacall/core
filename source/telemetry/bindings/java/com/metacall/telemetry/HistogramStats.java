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

/**
 * Statistics for histogram metrics
 */
public class HistogramStats {
	private final long count;
	private final double sum;
	private final double min;
	private final double max;

	public HistogramStats(long count, double sum, double min, double max) {
		this.count = count;
		this.sum = sum;
		this.min = min;
		this.max = max;
	}

	public long getCount() {
		return count;
	}

	public double getSum() {
		return sum;
	}

	public double getMin() {
		return min;
	}

	public double getMax() {
		return max;
	}

	public double getAverage() {
		return count > 0 ? sum / count : 0.0;
	}

	@Override
	public String toString() {
		return String.format("HistogramStats{count=%d, sum=%.6f, min=%.6f, max=%.6f, avg=%.6f}",
			count, sum, min, max, getAverage());
	}
}
