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

import java.util.concurrent.atomic.AtomicLong;

/**
 * Counter metric - monotonically increasing value
 */
public class Counter extends Metric {
	private final AtomicLong value;

	public Counter(String name) {
		this(name, "", "");
	}

	public Counter(String name, String description) {
		this(name, description, "");
	}

	public Counter(String name, String description, String unit) {
		super(name, description, unit);
		this.value = new AtomicLong(0);
	}

	/**
	 * Increment the counter by 1
	 */
	public void increment() {
		increment(1);
	}

	/**
	 * Increment the counter by a specific value
	 * @param delta The value to add
	 */
	public void increment(long delta) {
		// TODO: Call native C library function
		value.addAndGet(delta);
	}

	/**
	 * Get the current counter value
	 * @return The current value
	 */
	public long get() {
		// TODO: Call native C library function
		return value.get();
	}

	/**
	 * Reset the counter to zero
	 */
	public void reset() {
		// TODO: Call native C library function
		value.set(0);
	}
}
