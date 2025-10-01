#!/usr/bin/env python3
"""
Telemetry Library Python Tests
Test suite for Python bindings of the MetaCall telemetry system.

Copyright (C) 2025 MetaCall Inc., Dhiren Mhatre
"""

import sys
import os

# Add the bindings directory to the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../bindings/python'))

from telemetry import (
    Logger, LogLevel, LogHandler, LogFormatter, HandlerType, LogFormat,
    Counter, Gauge, Histogram, MetricRegistry, MetricExportFormat,
    get_logger, timed, counted
)


def test_logger_initialization():
    """Test logger initialization"""
    print("Testing logger initialization...")
    Logger.initialize()
    Logger.shutdown()
    print("✓ Logger initialization test passed")


def test_log_levels():
    """Test log level management"""
    print("Testing log levels...")
    Logger.initialize()

    Logger.set_level(LogLevel.DEBUG)
    assert Logger.get_level() == LogLevel.DEBUG

    Logger.set_level(LogLevel.ERROR)
    assert Logger.get_level() == LogLevel.ERROR

    Logger.shutdown()
    print("✓ Log levels test passed")


def test_logging_messages():
    """Test logging various messages"""
    print("Testing logging messages...")
    Logger.initialize()
    Logger.set_level(LogLevel.TRACE)

    Logger.trace("test", "This is a trace message")
    Logger.debug("test", "This is a debug message")
    Logger.info("test", "This is an info message")
    Logger.warning("test", "This is a warning message")
    Logger.error("test", "This is an error message")
    Logger.critical("test", "This is a critical message")

    Logger.shutdown()
    print("✓ Logging messages test passed")


def test_category_logger():
    """Test category-scoped logger"""
    print("Testing category logger...")
    Logger.initialize()

    logger = get_logger("myapp")
    logger.info("Starting application")
    logger.debug("Debug information")
    logger.error("An error occurred")

    Logger.shutdown()
    print("✓ Category logger test passed")


def test_counter_metric():
    """Test counter metrics"""
    print("Testing counter metrics...")
    MetricRegistry.initialize()

    counter = Counter("test_counter", "A test counter", "requests")
    MetricRegistry.register(counter)

    counter.increment()
    assert counter.get() == 1

    counter.increment(5)
    assert counter.get() == 6

    counter.reset()
    assert counter.get() == 0

    MetricRegistry.unregister(counter)
    MetricRegistry.shutdown()
    print("✓ Counter metrics test passed")


def test_gauge_metric():
    """Test gauge metrics"""
    print("Testing gauge metrics...")
    MetricRegistry.initialize()

    gauge = Gauge("test_gauge", "A test gauge", "bytes")
    MetricRegistry.register(gauge)

    gauge.set(100.0)
    assert gauge.get() == 100.0

    gauge.increment(50.0)
    assert gauge.get() == 150.0

    gauge.decrement(25.0)
    assert gauge.get() == 125.0

    MetricRegistry.unregister(gauge)
    MetricRegistry.shutdown()
    print("✓ Gauge metrics test passed")


def test_histogram_metric():
    """Test histogram metrics"""
    print("Testing histogram metrics...")
    MetricRegistry.initialize()

    histogram = Histogram("test_histogram", "A test histogram", "seconds",
                         [0.1, 0.5, 1.0, 5.0, 10.0])
    MetricRegistry.register(histogram)

    histogram.observe(0.05)
    histogram.observe(0.3)
    histogram.observe(0.8)
    histogram.observe(2.5)
    histogram.observe(7.0)

    stats = histogram.get_stats()
    assert stats['count'] == 5
    assert stats['sum'] == (0.05 + 0.3 + 0.8 + 2.5 + 7.0)
    assert stats['min'] == 0.05
    assert stats['max'] == 7.0

    histogram.reset()
    stats = histogram.get_stats()
    assert stats['count'] == 0

    MetricRegistry.unregister(histogram)
    MetricRegistry.shutdown()
    print("✓ Histogram metrics test passed")


def test_metric_labels():
    """Test metric labels"""
    print("Testing metric labels...")
    MetricRegistry.initialize()

    counter = Counter("http_requests_total", "Total HTTP requests", "requests")
    counter.add_label("method", "GET")
    counter.add_label("status", "200")
    counter.add_label("endpoint", "/api/users")

    counter.increment(42)
    MetricRegistry.register(counter)

    MetricRegistry.unregister(counter)
    MetricRegistry.shutdown()
    print("✓ Metric labels test passed")


def test_metric_registry():
    """Test metric registry operations"""
    print("Testing metric registry...")
    MetricRegistry.initialize()

    counter = Counter("registry_counter", "A counter in registry")
    gauge = Gauge("registry_gauge", "A gauge in registry")

    MetricRegistry.register(counter)
    MetricRegistry.register(gauge)

    found_counter = MetricRegistry.get("registry_counter")
    assert found_counter is not None
    assert found_counter == counter

    found_gauge = MetricRegistry.get("registry_gauge")
    assert found_gauge is not None
    assert found_gauge == gauge

    not_found = MetricRegistry.get("nonexistent")
    assert not_found is None

    all_metrics = MetricRegistry.get_all()
    assert len(all_metrics) >= 2

    MetricRegistry.unregister(counter)
    MetricRegistry.unregister(gauge)
    MetricRegistry.shutdown()
    print("✓ Metric registry test passed")


def test_prometheus_export():
    """Test Prometheus export format"""
    print("Testing Prometheus export...")
    MetricRegistry.initialize()

    counter = Counter("export_counter", "Counter for export test", "requests")
    counter.increment(100)
    MetricRegistry.register(counter)

    gauge = Gauge("export_gauge", "Gauge for export test", "bytes")
    gauge.set(1024.0)
    MetricRegistry.register(gauge)

    export_data = MetricRegistry.export_all(MetricExportFormat.PROMETHEUS)
    assert "export_counter" in export_data
    assert "export_gauge" in export_data
    assert "counter" in export_data
    assert "gauge" in export_data

    print("Prometheus export sample:")
    print(export_data[:200])

    MetricRegistry.unregister(counter)
    MetricRegistry.unregister(gauge)
    MetricRegistry.shutdown()
    print("✓ Prometheus export test passed")


def test_reset_all_metrics():
    """Test resetting all metrics"""
    print("Testing reset all metrics...")
    MetricRegistry.initialize()

    counter1 = Counter("reset_counter1")
    counter2 = Counter("reset_counter2")

    counter1.increment(10)
    counter2.increment(20)

    MetricRegistry.register(counter1)
    MetricRegistry.register(counter2)

    assert counter1.get() == 10
    assert counter2.get() == 20

    MetricRegistry.reset_all()

    assert counter1.get() == 0
    assert counter2.get() == 0

    MetricRegistry.unregister(counter1)
    MetricRegistry.unregister(counter2)
    MetricRegistry.shutdown()
    print("✓ Reset all metrics test passed")


def test_timed_decorator():
    """Test the timed decorator"""
    print("Testing timed decorator...")
    MetricRegistry.initialize()

    @timed("function_duration")
    def slow_function():
        import time
        time.sleep(0.1)
        return "done"

    result = slow_function()
    assert result == "done"

    histogram = MetricRegistry.get("function_duration")
    assert histogram is not None

    MetricRegistry.shutdown()
    print("✓ Timed decorator test passed")


def test_counted_decorator():
    """Test the counted decorator"""
    print("Testing counted decorator...")
    MetricRegistry.initialize()

    @counted("function_calls")
    def my_function():
        return "called"

    my_function()
    my_function()
    my_function()

    counter = MetricRegistry.get("function_calls")
    assert counter is not None
    assert counter.get() == 3

    MetricRegistry.shutdown()
    print("✓ Counted decorator test passed")


def run_all_tests():
    """Run all tests"""
    print("=== Python Telemetry Bindings Tests ===\n")

    test_logger_initialization()
    test_log_levels()
    test_logging_messages()
    test_category_logger()
    test_counter_metric()
    test_gauge_metric()
    test_histogram_metric()
    test_metric_labels()
    test_metric_registry()
    test_prometheus_export()
    test_reset_all_metrics()
    test_timed_decorator()
    test_counted_decorator()

    print("\n=== All Python Tests Passed ===")


if __name__ == "__main__":
    run_all_tests()
