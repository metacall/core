/**
 * Telemetry Library JavaScript Bindings
 * A JavaScript interface for the MetaCall telemetry and logging system.
 *
 * Copyright (C) 2025 MetaCall Inc., Dhiren Mhatre
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

'use strict';

// Log levels enum
const LogLevel = Object.freeze({
	TRACE: 0,
	DEBUG: 1,
	INFO: 2,
	WARNING: 3,
	ERROR: 4,
	CRITICAL: 5
});

// Log format types
const LogFormat = Object.freeze({
	TEXT: 0,
	JSON: 1,
	XML: 2,
	COLORED: 3
});

// Handler types
const HandlerType = Object.freeze({
	CONSOLE: 0,
	FILE: 1,
	SYSLOG: 2,
	NETWORK: 3,
	CUSTOM: 4
});

// Rotation policy
const RotationPolicy = Object.freeze({
	NONE: 0,
	SIZE: 1,
	TIME: 2,
	DAILY: 3,
	WEEKLY: 4,
	MONTHLY: 5
});

// Metric types
const MetricType = Object.freeze({
	COUNTER: 0,
	GAUGE: 1,
	HISTOGRAM: 2,
	SUMMARY: 3
});

// Metric export formats
const MetricExportFormat = Object.freeze({
	PROMETHEUS: 0,
	JSON: 1,
	STATSD: 2,
	INFLUXDB: 3,
	GRAPHITE: 4
});

// Telemetry error class
class TelemetryError extends Error {
	constructor(message) {
		super(message);
		this.name = 'TelemetryError';
	}
}

// Log handler class
class LogHandler {
	constructor(handlerType, minLevel) {
		this.handlerType = handlerType;
		this.minLevel = minLevel;
		this._handler = null;
		this._callback = null;
		this._formatter = null;
	}

	configureFile(filePath, rotation = RotationPolicy.NONE, maxSize = 0) {
		if (this.handlerType !== HandlerType.FILE) {
			throw new TelemetryError('configureFile only works with FILE handlers');
		}
		// TODO: Call native C library function
		this._filePath = filePath;
		this._rotation = rotation;
		this._maxSize = maxSize;
	}

	setCallback(callback) {
		if (typeof callback !== 'function') {
			throw new TelemetryError('Callback must be a function');
		}
		this._callback = callback;
		// TODO: Call native C library function
	}

	setFormatter(formatter) {
		if (!(formatter instanceof LogFormatter)) {
			throw new TelemetryError('Formatter must be an instance of LogFormatter');
		}
		this._formatter = formatter;
		// TODO: Call native C library function
	}

	register() {
		// TODO: Call native C library function
	}

	unregister() {
		// TODO: Call native C library function
	}
}

// Log formatter class
class LogFormatter {
	constructor(formatType) {
		this.formatType = formatType;
		this.includeTimestamp = true;
		this.includeLevel = true;
		this.includeCategory = true;
		this.includeLocation = true;
		this.includeThreadInfo = false;
		this._formatter = null;
	}

	configure(options = {}) {
		this.includeTimestamp = options.includeTimestamp !== undefined ? options.includeTimestamp : this.includeTimestamp;
		this.includeLevel = options.includeLevel !== undefined ? options.includeLevel : this.includeLevel;
		this.includeCategory = options.includeCategory !== undefined ? options.includeCategory : this.includeCategory;
		this.includeLocation = options.includeLocation !== undefined ? options.includeLocation : this.includeLocation;
		this.includeThreadInfo = options.includeThreadInfo !== undefined ? options.includeThreadInfo : this.includeThreadInfo;
		// TODO: Call native C library function
	}

	setCustomCallback(callback) {
		if (typeof callback !== 'function') {
			throw new TelemetryError('Callback must be a function');
		}
		// TODO: Call native C library function
	}
}

// Logger class
class Logger {
	static _initialized = false;
	static _handlers = [];
	static _level = LogLevel.INFO;

	static initialize() {
		if (!Logger._initialized) {
			// TODO: Call native C library telemetry_log_initialize()
			Logger._initialized = true;
		}
	}

	static shutdown() {
		if (Logger._initialized) {
			// TODO: Call native C library telemetry_log_shutdown()
			Logger._initialized = false;
		}
	}

	static setLevel(level) {
		Logger.initialize();
		Logger._level = level;
		// TODO: Call native C library function
	}

	static getLevel() {
		Logger.initialize();
		return Logger._level;
	}

	static trace(category, message) {
		Logger._log(LogLevel.TRACE, category, message);
	}

	static debug(category, message) {
		Logger._log(LogLevel.DEBUG, category, message);
	}

	static info(category, message) {
		Logger._log(LogLevel.INFO, category, message);
	}

	static warning(category, message) {
		Logger._log(LogLevel.WARNING, category, message);
	}

	static error(category, message) {
		Logger._log(LogLevel.ERROR, category, message);
	}

	static critical(category, message) {
		Logger._log(LogLevel.CRITICAL, category, message);
	}

	static _log(level, category, message) {
		Logger.initialize();

		if (level < Logger._level) {
			return;
		}

		// TODO: Call native C library telemetry_log_write()
		// Fallback to console for now
		const timestamp = new Date().toISOString();
		const levelStr = Object.keys(LogLevel).find(key => LogLevel[key] === level);
		const logMessage = `[${timestamp}] [${levelStr}] [${category}] ${message}`;

		if (level >= LogLevel.ERROR) {
			console.error(logMessage);
		} else if (level >= LogLevel.WARNING) {
			console.warn(logMessage);
		} else if (level >= LogLevel.INFO) {
			console.info(logMessage);
		} else {
			console.log(logMessage);
		}
	}

	static addHandler(handler) {
		if (!(handler instanceof LogHandler)) {
			throw new TelemetryError('Handler must be an instance of LogHandler');
		}
		Logger._handlers.push(handler);
		handler.register();
	}

	static removeHandler(handler) {
		const index = Logger._handlers.indexOf(handler);
		if (index !== -1) {
			handler.unregister();
			Logger._handlers.splice(index, 1);
		}
	}

	static flush() {
		Logger.initialize();
		// TODO: Call native C library telemetry_log_flush()
	}
}

// Base metric class
class Metric {
	constructor(name, description = '', unit = '') {
		this.name = name;
		this.description = description;
		this.unit = unit;
		this.labels = new Map();
		this._metric = null;
	}

	addLabel(key, value) {
		this.labels.set(key, value);
		// TODO: Call native C library function
	}

	register() {
		// TODO: Call native C library function
	}

	unregister() {
		// TODO: Call native C library function
	}

	getLabelsString() {
		if (this.labels.size === 0) {
			return '';
		}

		const labelPairs = Array.from(this.labels.entries())
			.map(([k, v]) => `${k}="${v}"`)
			.join(',');
		return `{${labelPairs}}`;
	}
}

// Counter metric
class Counter extends Metric {
	constructor(name, description = '', unit = '') {
		super(name, description, unit);
		this._value = 0;
	}

	increment(value = 1) {
		// TODO: Call native C library function
		this._value += value;
	}

	get() {
		// TODO: Call native C library function
		return this._value;
	}

	reset() {
		// TODO: Call native C library function
		this._value = 0;
	}
}

// Gauge metric
class Gauge extends Metric {
	constructor(name, description = '', unit = '') {
		super(name, description, unit);
		this._value = 0.0;
		this._min = Infinity;
		this._max = -Infinity;
	}

	set(value) {
		// TODO: Call native C library function
		this._value = value;
		this._min = Math.min(this._min, value);
		this._max = Math.max(this._max, value);
	}

	increment(value = 1.0) {
		this.set(this._value + value);
	}

	decrement(value = 1.0) {
		this.set(this._value - value);
	}

	get() {
		// TODO: Call native C library function
		return this._value;
	}

	getMin() {
		return this._min;
	}

	getMax() {
		return this._max;
	}
}

// Histogram metric
class Histogram extends Metric {
	constructor(name, description = '', unit = '', buckets = null) {
		super(name, description, unit);
		this.buckets = buckets || [0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 5.0, 10.0];
		this._observations = [];
		this._bucketCounts = new Array(this.buckets.length).fill(0);
	}

	observe(value) {
		// TODO: Call native C library function
		this._observations.push(value);

		// Update bucket counts
		for (let i = 0; i < this.buckets.length; i++) {
			if (value <= this.buckets[i]) {
				this._bucketCounts[i]++;
			}
		}
	}

	getStats() {
		// TODO: Call native C library function
		if (this._observations.length === 0) {
			return { count: 0, sum: 0, min: 0, max: 0 };
		}

		return {
			count: this._observations.length,
			sum: this._observations.reduce((a, b) => a + b, 0),
			min: Math.min(...this._observations),
			max: Math.max(...this._observations)
		};
	}

	getBuckets() {
		return this.buckets.map((bound, idx) => ({
			upperBound: bound,
			count: this._bucketCounts[idx]
		}));
	}

	reset() {
		// TODO: Call native C library function
		this._observations = [];
		this._bucketCounts.fill(0);
	}
}

// Metric registry
class MetricRegistry {
	static _initialized = false;
	static _metrics = new Map();

	static initialize() {
		if (!MetricRegistry._initialized) {
			// TODO: Call native C library telemetry_metrics_initialize()
			MetricRegistry._initialized = true;
		}
	}

	static shutdown() {
		if (MetricRegistry._initialized) {
			// TODO: Call native C library telemetry_metrics_shutdown()
			MetricRegistry._initialized = false;
		}
	}

	static register(metric) {
		MetricRegistry.initialize();

		if (!(metric instanceof Metric)) {
			throw new TelemetryError('Must register a Metric instance');
		}

		if (MetricRegistry._metrics.has(metric.name)) {
			throw new TelemetryError(`Metric '${metric.name}' already registered`);
		}

		MetricRegistry._metrics.set(metric.name, metric);
		metric.register();
	}

	static unregister(metric) {
		if (MetricRegistry._metrics.has(metric.name)) {
			metric.unregister();
			MetricRegistry._metrics.delete(metric.name);
		}
	}

	static get(name) {
		return MetricRegistry._metrics.get(name);
	}

	static getAll() {
		return Array.from(MetricRegistry._metrics.values());
	}

	static exportAll(format = MetricExportFormat.PROMETHEUS) {
		MetricRegistry.initialize();
		// TODO: Call native C library function

		// Fallback implementation for Prometheus format
		if (format === MetricExportFormat.PROMETHEUS) {
			const lines = [];

			for (const metric of MetricRegistry._metrics.values()) {
				if (metric.description) {
					lines.push(`# HELP ${metric.name} ${metric.description}`);
				}

				if (metric instanceof Counter) {
					lines.push(`# TYPE ${metric.name} counter`);
					lines.push(`${metric.name}${metric.getLabelsString()} ${metric.get()}`);
				} else if (metric instanceof Gauge) {
					lines.push(`# TYPE ${metric.name} gauge`);
					lines.push(`${metric.name}${metric.getLabelsString()} ${metric.get()}`);
				} else if (metric instanceof Histogram) {
					lines.push(`# TYPE ${metric.name} histogram`);
					const stats = metric.getStats();
					const buckets = metric.getBuckets();

					for (const bucket of buckets) {
						const labels = metric.getLabelsString();
						const bucketLabel = labels ? `{le="${bucket.upperBound}",${labels.slice(1)}` : `{le="${bucket.upperBound}"}`;
						lines.push(`${metric.name}_bucket${bucketLabel} ${bucket.count}`);
					}

					lines.push(`${metric.name}_sum${metric.getLabelsString()} ${stats.sum}`);
					lines.push(`${metric.name}_count${metric.getLabelsString()} ${stats.count}`);
				}

				lines.push('');
			}

			return lines.join('\n');
		} else if (format === MetricExportFormat.JSON) {
			const metrics = [];

			for (const metric of MetricRegistry._metrics.values()) {
				const data = {
					name: metric.name,
					description: metric.description,
					unit: metric.unit,
					labels: Object.fromEntries(metric.labels)
				};

				if (metric instanceof Counter) {
					data.type = 'counter';
					data.value = metric.get();
				} else if (metric instanceof Gauge) {
					data.type = 'gauge';
					data.value = metric.get();
					data.min = metric.getMin();
					data.max = metric.getMax();
				} else if (metric instanceof Histogram) {
					data.type = 'histogram';
					data.stats = metric.getStats();
					data.buckets = metric.getBuckets();
				}

				metrics.push(data);
			}

			return JSON.stringify(metrics, null, 2);
		}

		return '';
	}

	static resetAll() {
		MetricRegistry.initialize();
		// TODO: Call native C library function

		for (const metric of MetricRegistry._metrics.values()) {
			if (typeof metric.reset === 'function') {
				metric.reset();
			}
		}
	}
}

// Metric exporter
class MetricExporter {
	constructor(format, endpoint, pushInterval = 0) {
		this.format = format;
		this.endpoint = endpoint;
		this.pushInterval = pushInterval;
		this._exporter = null;
		this._intervalHandle = null;
	}

	register() {
		// TODO: Call native C library function

		if (this.pushInterval > 0) {
			this._intervalHandle = setInterval(() => {
				this.export();
			}, this.pushInterval * 1000);
		}
	}

	unregister() {
		// TODO: Call native C library function

		if (this._intervalHandle) {
			clearInterval(this._intervalHandle);
			this._intervalHandle = null;
		}
	}

	export() {
		// TODO: Call native C library function
		const data = MetricRegistry.exportAll(this.format);

		// In a real implementation, this would send to endpoint
		// For now, just log it
		console.log(`Exporting to ${this.endpoint}:`);
		console.log(data);

		return data;
	}
}

// Category logger helper
class CategoryLogger {
	constructor(category) {
		this.category = category;
	}

	trace(message) {
		Logger.trace(this.category, message);
	}

	debug(message) {
		Logger.debug(this.category, message);
	}

	info(message) {
		Logger.info(this.category, message);
	}

	warning(message) {
		Logger.warning(this.category, message);
	}

	error(message) {
		Logger.error(this.category, message);
	}

	critical(message) {
		Logger.critical(this.category, message);
	}
}

// Utility function to get a category logger
function getLogger(category) {
	return new CategoryLogger(category);
}

// Decorators (using function wrappers)
function timed(metricName) {
	return function(target, propertyKey, descriptor) {
		const originalMethod = descriptor.value;

		descriptor.value = function(...args) {
			let histogram = MetricRegistry.get(metricName);
			if (!histogram) {
				histogram = new Histogram(metricName, `Execution time of ${propertyKey}`, 'seconds');
				MetricRegistry.register(histogram);
			}

			const start = Date.now();
			try {
				return originalMethod.apply(this, args);
			} finally {
				const duration = (Date.now() - start) / 1000;
				histogram.observe(duration);
			}
		};

		return descriptor;
	};
}

function counted(metricName) {
	return function(target, propertyKey, descriptor) {
		const originalMethod = descriptor.value;

		descriptor.value = function(...args) {
			let counter = MetricRegistry.get(metricName);
			if (!counter) {
				counter = new Counter(metricName, `Call count of ${propertyKey}`, 'calls');
				MetricRegistry.register(counter);
			}

			counter.increment();
			return originalMethod.apply(this, args);
		};

		return descriptor;
	};
}

// Initialize on load
Logger.initialize();
MetricRegistry.initialize();

// Export everything
module.exports = {
	// Enums
	LogLevel,
	LogFormat,
	HandlerType,
	RotationPolicy,
	MetricType,
	MetricExportFormat,

	// Classes
	TelemetryError,
	Logger,
	LogHandler,
	LogFormatter,
	Metric,
	Counter,
	Gauge,
	Histogram,
	MetricRegistry,
	MetricExporter,
	CategoryLogger,

	// Utility functions
	getLogger,
	timed,
	counted
};
