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
 * Log severity levels
 */
public enum LogLevel {
	TRACE(0),
	DEBUG(1),
	INFO(2),
	WARNING(3),
	ERROR(4),
	CRITICAL(5);

	private final int value;

	LogLevel(int value) {
		this.value = value;
	}

	public int getValue() {
		return value;
	}

	public static LogLevel fromValue(int value) {
		for (LogLevel level : values()) {
			if (level.value == value) {
				return level;
			}
		}
		return INFO;
	}
}
