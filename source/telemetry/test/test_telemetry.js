/**
 * Telemetry Library JavaScript Tests
 * Test suite for JavaScript bindings of the MetaCall telemetry system.
 *
 * Copyright (C) 2025 MetaCall Inc., Dhiren Mhatre
 */

'use strict';

const telemetry = require('../bindings/javascript/telemetry');

const {
	Logger,
	LogLevel,
	LogHandler,
	LogFormatter,
	HandlerType,
	LogFormat,
	Counter,
	Gauge,
	Histogram,
	MetricRegistry,
	MetricExportFormat,
	getLogger
} = telemetry;

let testsPassed = 0;
let testsFailed = 0;

function assert(condition, message) {
	if (!condition) {
		console.error(`✗ Assertion failed: ${message}`);
		testsFailed++;
		throw new Error(`Assertion failed: ${message}`);
	}
}

function assertEquals(actual, expected, message) {
	if (actual !== expected) {
		console.error(`✗ ${message}: expected ${expected}, got ${actual}`);
		testsFailed++;
		throw new Error(`Expected ${expected}, got ${actual}`);
	}
}

function test(name, fn) {
	try {
		console.log(`Testing ${name}...`);
		fn();
		console.log(`✓ ${name} test passed`);
		testsPassed++;
	} catch (error) {
		console.error(`✗ ${name} test failed:`, error.message);
		testsFailed++;
	}
}

// Test logger initialization
test('logger initialization', () => {
	Logger.initialize();
	Logger.shutdown();
});

// Test log levels
test('log levels', () => {
	Logger.initialize();

	Logger.setLevel(LogLevel.DEBUG);
	assertEquals(Logger.getLevel(), LogLevel.DEBUG, "Log level should be DEBUG");

	Logger.setLevel(LogLevel.ERROR);
	assertEquals(Logger.getLevel(), LogLevel.ERROR, "Log level should be ERROR");

	Logger.shutdown();
});

// Test logging messages
test('logging messages', () => {
	Logger.initialize();
	Logger.setLevel(LogLevel.TRACE);

	Logger.trace("test", "This is a trace message");
	Logger.debug("test", "This is a debug message");
	Logger.info("test", "This is an info message");
	Logger.warning("test", "This is a warning message");
	Logger.error("test", "This is an error message");
	Logger.critical("test", "This is a critical message");

	Logger.shutdown();
});

// Test category logger
test('category logger', () => {
	Logger.initialize();

	const logger = getLogger("myapp");
	logger.info("Starting application");
	logger.debug("Debug information");
	logger.error("An error occurred");

	Logger.shutdown();
});

// Test log handler
test('log handler', () => {
	Logger.initialize();

	const handler = new LogHandler(HandlerType.CONSOLE, LogLevel.INFO);
	Logger.addHandler(handler);
	Logger.removeHandler(handler);

	Logger.shutdown();
});

// Test log formatter
test('log formatter', () => {
	Logger.initialize();

	const formatter = new LogFormatter(LogFormat.TEXT);
	formatter.configure({
		includeTimestamp: true,
		includeLevel: true,
		includeCategory: true,
		includeLocation: false,
		includeThreadInfo: false
	});

	Logger.shutdown();
});

// Test counter metric
test('counter metric', () => {
	MetricRegistry.initialize();

	const counter = new Counter("test_counter", "A test counter", "requests");
	MetricRegistry.register(counter);

	counter.increment();
	assertEquals(counter.get(), 1, "Counter should be 1");

	counter.increment(5);
	assertEquals(counter.get(), 6, "Counter should be 6");

	counter.reset();
	assertEquals(counter.get(), 0, "Counter should be 0 after reset");

	MetricRegistry.unregister(counter);
	MetricRegistry.shutdown();
});

// Test gauge metric
test('gauge metric', () => {
	MetricRegistry.initialize();

	const gauge = new Gauge("test_gauge", "A test gauge", "bytes");
	MetricRegistry.register(gauge);

	gauge.set(100.0);
	assertEquals(gauge.get(), 100.0, "Gauge should be 100.0");

	gauge.increment(50.0);
	assertEquals(gauge.get(), 150.0, "Gauge should be 150.0");

	gauge.decrement(25.0);
	assertEquals(gauge.get(), 125.0, "Gauge should be 125.0");

	MetricRegistry.unregister(gauge);
	MetricRegistry.shutdown();
});

// Test histogram metric
test('histogram metric', () => {
	MetricRegistry.initialize();

	const histogram = new Histogram("test_histogram", "A test histogram", "seconds",
		[0.1, 0.5, 1.0, 5.0, 10.0]);
	MetricRegistry.register(histogram);

	histogram.observe(0.05);
	histogram.observe(0.3);
	histogram.observe(0.8);
	histogram.observe(2.5);
	histogram.observe(7.0);

	const stats = histogram.getStats();
	assertEquals(stats.count, 5, "Histogram count should be 5");
	assertEquals(stats.sum, 10.65, "Histogram sum should be 10.65");
	assertEquals(stats.min, 0.05, "Histogram min should be 0.05");
	assertEquals(stats.max, 7.0, "Histogram max should be 7.0");

	histogram.reset();
	const resetStats = histogram.getStats();
	assertEquals(resetStats.count, 0, "Histogram count should be 0 after reset");

	MetricRegistry.unregister(histogram);
	MetricRegistry.shutdown();
});

// Test metric labels
test('metric labels', () => {
	MetricRegistry.initialize();

	const counter = new Counter("http_requests_total", "Total HTTP requests", "requests");
	counter.addLabel("method", "GET");
	counter.addLabel("status", "200");
	counter.addLabel("endpoint", "/api/users");

	counter.increment(42);
	MetricRegistry.register(counter);

	const labelsString = counter.getLabelsString();
	assert(labelsString.includes("method"), "Labels should include method");
	assert(labelsString.includes("GET"), "Labels should include GET");

	MetricRegistry.unregister(counter);
	MetricRegistry.shutdown();
});

// Test metric registry
test('metric registry', () => {
	MetricRegistry.initialize();

	const counter = new Counter("registry_counter", "A counter in registry");
	const gauge = new Gauge("registry_gauge", "A gauge in registry");

	MetricRegistry.register(counter);
	MetricRegistry.register(gauge);

	const foundCounter = MetricRegistry.get("registry_counter");
	assert(foundCounter !== null, "Should find counter");
	assertEquals(foundCounter, counter, "Found counter should match");

	const foundGauge = MetricRegistry.get("registry_gauge");
	assert(foundGauge !== null, "Should find gauge");
	assertEquals(foundGauge, gauge, "Found gauge should match");

	const notFound = MetricRegistry.get("nonexistent");
	assert(notFound === undefined, "Should not find nonexistent metric");

	const allMetrics = MetricRegistry.getAll();
	assert(allMetrics.length >= 2, "Should have at least 2 metrics");

	MetricRegistry.unregister(counter);
	MetricRegistry.unregister(gauge);
	MetricRegistry.shutdown();
});

// Test Prometheus export
test('Prometheus export', () => {
	MetricRegistry.initialize();

	const counter = new Counter("export_counter", "Counter for export test", "requests");
	counter.increment(100);
	MetricRegistry.register(counter);

	const gauge = new Gauge("export_gauge", "Gauge for export test", "bytes");
	gauge.set(1024.0);
	MetricRegistry.register(gauge);

	const exportData = MetricRegistry.exportAll(MetricExportFormat.PROMETHEUS);
	assert(exportData.includes("export_counter"), "Export should include counter");
	assert(exportData.includes("export_gauge"), "Export should include gauge");
	assert(exportData.includes("counter"), "Export should include counter type");
	assert(exportData.includes("gauge"), "Export should include gauge type");

	console.log("Prometheus export sample:");
	console.log(exportData.substring(0, 200));

	MetricRegistry.unregister(counter);
	MetricRegistry.unregister(gauge);
	MetricRegistry.shutdown();
});

// Test JSON export
test('JSON export', () => {
	MetricRegistry.initialize();

	const counter = new Counter("json_counter", "Counter for JSON test", "requests");
	counter.increment(50);
	MetricRegistry.register(counter);

	const exportData = MetricRegistry.exportAll(MetricExportFormat.JSON);
	assert(exportData.includes("json_counter"), "Export should include counter name");

	const parsed = JSON.parse(exportData);
	assert(Array.isArray(parsed), "Parsed data should be an array");
	assert(parsed.length > 0, "Should have at least one metric");

	MetricRegistry.unregister(counter);
	MetricRegistry.shutdown();
});

// Test reset all metrics
test('reset all metrics', () => {
	MetricRegistry.initialize();

	const counter1 = new Counter("reset_counter1");
	const counter2 = new Counter("reset_counter2");

	counter1.increment(10);
	counter2.increment(20);

	MetricRegistry.register(counter1);
	MetricRegistry.register(counter2);

	assertEquals(counter1.get(), 10, "Counter1 should be 10");
	assertEquals(counter2.get(), 20, "Counter2 should be 20");

	MetricRegistry.resetAll();

	assertEquals(counter1.get(), 0, "Counter1 should be 0 after reset");
	assertEquals(counter2.get(), 0, "Counter2 should be 0 after reset");

	MetricRegistry.unregister(counter1);
	MetricRegistry.unregister(counter2);
	MetricRegistry.shutdown();
});

// Test metric exporter
test('metric exporter', () => {
	MetricRegistry.initialize();

	const exporter = new telemetry.MetricExporter(
		MetricExportFormat.PROMETHEUS,
		"/tmp/metrics.txt",
		60
	);

	exporter.register();
	exporter.unregister();

	MetricRegistry.shutdown();
});

// Test histogram buckets
test('histogram buckets', () => {
	MetricRegistry.initialize();

	const histogram = new Histogram("bucket_test", "Test buckets", "ms");
	histogram.observe(0.1);
	histogram.observe(1.5);
	histogram.observe(8.0);

	const buckets = histogram.getBuckets();
	assert(buckets.length > 0, "Should have buckets");

	MetricRegistry.unregister(histogram);
	MetricRegistry.shutdown();
});

// Test gauge min/max tracking
test('gauge min/max tracking', () => {
	MetricRegistry.initialize();

	const gauge = new Gauge("minmax_gauge", "Test min/max");
	MetricRegistry.register(gauge);

	gauge.set(100);
	gauge.set(50);
	gauge.set(150);
	gauge.set(75);

	assertEquals(gauge.getMin(), 50, "Min should be 50");
	assertEquals(gauge.getMax(), 150, "Max should be 150");
	assertEquals(gauge.get(), 75, "Current value should be 75");

	MetricRegistry.unregister(gauge);
	MetricRegistry.shutdown();
});

// Run all tests
console.log("=== JavaScript Telemetry Bindings Tests ===\n");

// Allow tests to complete
setTimeout(() => {
	console.log(`\n=== Test Results ===`);
	console.log(`Tests passed: ${testsPassed}`);
	console.log(`Tests failed: ${testsFailed}`);

	if (testsFailed === 0) {
		console.log("\n=== All JavaScript Tests Passed ===");
		process.exit(0);
	} else {
		console.log("\n=== Some Tests Failed ===");
		process.exit(1);
	}
}, 100);
