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
 * Logger scoped to a specific category
 */
public class CategoryLogger {
	private final String category;

	public CategoryLogger(String category) {
		this.category = category;
	}

	public String getCategory() {
		return category;
	}

	public void trace(String message) {
		Logger.trace(category, message);
	}

	public void debug(String message) {
		Logger.debug(category, message);
	}

	public void info(String message) {
		Logger.info(category, message);
	}

	public void warning(String message) {
		Logger.warning(category, message);
	}

	public void error(String message) {
		Logger.error(category, message);
	}

	public void critical(String message) {
		Logger.critical(category, message);
	}
}
