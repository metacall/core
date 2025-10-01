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

/**
 * Base class for all metrics
 */
public abstract class Metric {
	protected final String name;
	protected final String description;
	protected final String unit;
	protected final Map<String, String> labels;

	protected Metric(String name, String description, String unit) {
		this.name = name;
		this.description = description != null ? description : "";
		this.unit = unit != null ? unit : "";
		this.labels = new HashMap<>();
	}

	public String getName() {
		return name;
	}

	public String getDescription() {
		return description;
	}

	public String getUnit() {
		return unit;
	}

	public void addLabel(String key, String value) {
		labels.put(key, value);
		// TODO: Call native C library function
	}

	public Map<String, String> getLabels() {
		return new HashMap<>(labels);
	}

	public String getLabelsString() {
		if (labels.isEmpty()) {
			return "";
		}

		StringBuilder sb = new StringBuilder("{");
		boolean first = true;
		for (Map.Entry<String, String> entry : labels.entrySet()) {
			if (!first) {
				sb.append(",");
			}
			sb.append(entry.getKey()).append("=\"").append(entry.getValue()).append("\"");
			first = false;
		}
		sb.append("}");
		return sb.toString();
	}

	protected void register() {
		// TODO: Call native C library function
	}

	protected void unregister() {
		// TODO: Call native C library function
	}
}
