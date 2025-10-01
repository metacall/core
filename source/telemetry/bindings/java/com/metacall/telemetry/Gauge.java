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

import java.util.concurrent.atomic.AtomicReference;

/**
 * Gauge metric - value that can go up and down
 */
public class Gauge extends Metric {
	private final AtomicReference<Double> value;
	private volatile double minValue;
	private volatile double maxValue;

	public Gauge(String name) {
		this(name, "", "");
	}

	public Gauge(String name, String description) {
		this(name, description, "");
	}

	public Gauge(String name, String description, String unit) {
		super(name, description, unit);
		this.value = new AtomicReference<>(0.0);
		this.minValue = Double.MAX_VALUE;
		this.maxValue = -Double.MAX_VALUE;
	}

	/**
	 * Set the gauge value
	 * @param newValue The new value
	 */
	public void set(double newValue) {
		// TODO: Call native C library function
		value.set(newValue);
		updateMinMax(newValue);
	}

	/**
	 * Increment the gauge by 1.0
	 */
	public void increment() {
		increment(1.0);
	}

	/**
	 * Increment the gauge by a specific value
	 * @param delta The value to add
	 */
	public void increment(double delta) {
		double newValue;
		do {
			double current = value.get();
			newValue = current + delta;
		} while (!value.compareAndSet(value.get(), newValue));
		updateMinMax(newValue);
	}

	/**
	 * Decrement the gauge by 1.0
	 */
	public void decrement() {
		decrement(1.0);
	}

	/**
	 * Decrement the gauge by a specific value
	 * @param delta The value to subtract
	 */
	public void decrement(double delta) {
		increment(-delta);
	}

	/**
	 * Get the current gauge value
	 * @return The current value
	 */
	public double get() {
		// TODO: Call native C library function
		return value.get();
	}

	/**
	 * Get the minimum observed value
	 * @return The minimum value
	 */
	public double getMin() {
		return minValue;
	}

	/**
	 * Get the maximum observed value
	 * @return The maximum value
	 */
	public double getMax() {
		return maxValue;
	}

	private void updateMinMax(double newValue) {
		if (newValue < minValue) {
			minValue = newValue;
		}
		if (newValue > maxValue) {
			maxValue = newValue;
		}
	}
}
