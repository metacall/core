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
 * Log handler for processing log entries
 */
public class LogHandler {
	private final HandlerType handlerType;
	private final LogLevel minLevel;
	private LogFormatter formatter;
	private String filePath;
	private RotationPolicy rotation;
	private long maxFileSize;

	public LogHandler(HandlerType handlerType, LogLevel minLevel) {
		this.handlerType = handlerType;
		this.minLevel = minLevel;
		this.rotation = RotationPolicy.NONE;
		this.maxFileSize = 0;
	}

	public void configureFile(String filePath, RotationPolicy rotation, long maxSize) {
		if (handlerType != HandlerType.FILE) {
			throw new IllegalStateException("configureFile only works with FILE handlers");
		}
		this.filePath = filePath;
		this.rotation = rotation;
		this.maxFileSize = maxSize;
		// TODO: Call native C library function
	}

	public void setFormatter(LogFormatter formatter) {
		this.formatter = formatter;
		// TODO: Call native C library function
	}

	public LogFormatter getFormatter() {
		return formatter;
	}

	public HandlerType getHandlerType() {
		return handlerType;
	}

	public LogLevel getMinLevel() {
		return minLevel;
	}

	public String getFilePath() {
		return filePath;
	}

	public RotationPolicy getRotation() {
		return rotation;
	}

	public long getMaxFileSize() {
		return maxFileSize;
	}

	protected void register() {
		// TODO: Call native C library function
	}

	protected void unregister() {
		// TODO: Call native C library function
	}
}
