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

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Main logging interface for the telemetry system
 */
public class Logger {
	private static boolean initialized = false;
	private static LogLevel globalLevel = LogLevel.INFO;
	private static final List<LogHandler> handlers = new ArrayList<>();
	private static final ReentrantLock lock = new ReentrantLock();

	/**
	 * Initialize the logging system
	 */
	public static void initialize() {
		if (!initialized) {
			lock.lock();
			try {
				if (!initialized) {
					// TODO: Call native C library telemetry_log_initialize()
					initialized = true;
				}
			} finally {
				lock.unlock();
			}
		}
	}

	/**
	 * Shutdown the logging system
	 */
	public static void shutdown() {
		if (initialized) {
			lock.lock();
			try {
				if (initialized) {
					// TODO: Call native C library telemetry_log_shutdown()
					handlers.clear();
					initialized = false;
				}
			} finally {
				lock.unlock();
			}
		}
	}

	/**
	 * Set the global log level
	 * @param level The minimum log level to process
	 */
	public static void setLevel(LogLevel level) {
		initialize();
		lock.lock();
		try {
			globalLevel = level;
			// TODO: Call native C library function
		} finally {
			lock.unlock();
		}
	}

	/**
	 * Get the global log level
	 * @return The current global log level
	 */
	public static LogLevel getLevel() {
		initialize();
		return globalLevel;
	}

	/**
	 * Log a trace message
	 * @param category Log category
	 * @param message Log message
	 */
	public static void trace(String category, String message) {
		log(LogLevel.TRACE, category, message);
	}

	/**
	 * Log a debug message
	 * @param category Log category
	 * @param message Log message
	 */
	public static void debug(String category, String message) {
		log(LogLevel.DEBUG, category, message);
	}

	/**
	 * Log an info message
	 * @param category Log category
	 * @param message Log message
	 */
	public static void info(String category, String message) {
		log(LogLevel.INFO, category, message);
	}

	/**
	 * Log a warning message
	 * @param category Log category
	 * @param message Log message
	 */
	public static void warning(String category, String message) {
		log(LogLevel.WARNING, category, message);
	}

	/**
	 * Log an error message
	 * @param category Log category
	 * @param message Log message
	 */
	public static void error(String category, String message) {
		log(LogLevel.ERROR, category, message);
	}

	/**
	 * Log a critical message
	 * @param category Log category
	 * @param message Log message
	 */
	public static void critical(String category, String message) {
		log(LogLevel.CRITICAL, category, message);
	}

	/**
	 * Internal logging method
	 * @param level Log level
	 * @param category Log category
	 * @param message Log message
	 */
	private static void log(LogLevel level, String category, String message) {
		initialize();

		if (level.ordinal() < globalLevel.ordinal()) {
			return;
		}

		// TODO: Call native C library telemetry_log_write()
		// Fallback to System.out/err for now
		String timestamp = java.time.Instant.now().toString();
		String logMessage = String.format("[%s] [%s] [%s] %s", timestamp, level, category, message);

		if (level.ordinal() >= LogLevel.ERROR.ordinal()) {
			System.err.println(logMessage);
		} else {
			System.out.println(logMessage);
		}
	}

	/**
	 * Add a log handler
	 * @param handler The handler to add
	 */
	public static void addHandler(LogHandler handler) {
		initialize();
		lock.lock();
		try {
			handlers.add(handler);
			handler.register();
		} finally {
			lock.unlock();
		}
	}

	/**
	 * Remove a log handler
	 * @param handler The handler to remove
	 */
	public static void removeHandler(LogHandler handler) {
		lock.lock();
		try {
			if (handlers.remove(handler)) {
				handler.unregister();
			}
		} finally {
			lock.unlock();
		}
	}

	/**
	 * Flush all pending log entries
	 */
	public static void flush() {
		initialize();
		// TODO: Call native C library telemetry_log_flush()
	}

	/**
	 * Get a category-scoped logger
	 * @param category The category name
	 * @return A CategoryLogger instance
	 */
	public static CategoryLogger getLogger(String category) {
		return new CategoryLogger(category);
	}
}
