"""
Telemetry Library Python Bindings
A Python interface for the MetaCall telemetry and logging system.

Copyright (C) 2025 MetaCall Inc., Dhiren Mhatre

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
"""

import ctypes
import enum
import os
import sys
from typing import Any, Callable, Dict, List, Optional, Union


class LogLevel(enum.IntEnum):
    """Log severity levels"""
    TRACE = 0
    DEBUG = 1
    INFO = 2
    WARNING = 3
    ERROR = 4
    CRITICAL = 5


class LogFormat(enum.IntEnum):
    """Log output format types"""
    TEXT = 0
    JSON = 1
    XML = 2
    COLORED = 3


class HandlerType(enum.IntEnum):
    """Log handler types"""
    CONSOLE = 0
    FILE = 1
    SYSLOG = 2
    NETWORK = 3
    CUSTOM = 4


class RotationPolicy(enum.IntEnum):
    """Log rotation policy"""
    NONE = 0
    SIZE = 1
    TIME = 2
    DAILY = 3
    WEEKLY = 4
    MONTHLY = 5


class MetricType(enum.IntEnum):
    """Metric types"""
    COUNTER = 0
    GAUGE = 1
    HISTOGRAM = 2
    SUMMARY = 3


class MetricExportFormat(enum.IntEnum):
    """Metric export format types"""
    PROMETHEUS = 0
    JSON = 1
    STATSD = 2
    INFLUXDB = 3
    GRAPHITE = 4


class TelemetryError(Exception):
    """Base exception for telemetry errors"""
    pass


class LogHandler:
    """Python wrapper for telemetry log handler"""

    def __init__(self, handler_type: HandlerType, min_level: LogLevel):
        self.handler_type = handler_type
        self.min_level = min_level
        self._handler = None
        self._callback = None

    def configure_file(self, file_path: str, rotation: RotationPolicy = RotationPolicy.NONE, max_size: int = 0):
        """Configure file handler settings"""
        if self.handler_type != HandlerType.FILE:
            raise TelemetryError("configure_file only works with FILE handlers")
        # TODO: Call C library function
        pass

    def set_callback(self, callback: Callable):
        """Set custom handler callback"""
        self._callback = callback
        # TODO: Call C library function
        pass

    def register(self):
        """Register this handler with the logging system"""
        # TODO: Call C library function
        pass

    def unregister(self):
        """Unregister this handler from the logging system"""
        # TODO: Call C library function
        pass


class LogFormatter:
    """Python wrapper for telemetry log formatter"""

    def __init__(self, format_type: LogFormat):
        self.format_type = format_type
        self.include_timestamp = True
        self.include_level = True
        self.include_category = True
        self.include_location = True
        self.include_thread_info = False
        self._formatter = None

    def configure(self, include_timestamp: bool = True, include_level: bool = True,
                 include_category: bool = True, include_location: bool = True,
                 include_thread_info: bool = False):
        """Configure formatter options"""
        self.include_timestamp = include_timestamp
        self.include_level = include_level
        self.include_category = include_category
        self.include_location = include_location
        self.include_thread_info = include_thread_info
        # TODO: Call C library function
        pass


class Logger:
    """Main logging interface"""

    _initialized = False
    _handlers: List[LogHandler] = []

    @classmethod
    def initialize(cls):
        """Initialize the logging system"""
        if not cls._initialized:
            # TODO: Call C library telemetry_log_initialize()
            cls._initialized = True

    @classmethod
    def shutdown(cls):
        """Shutdown the logging system"""
        if cls._initialized:
            # TODO: Call C library telemetry_log_shutdown()
            cls._initialized = False

    @classmethod
    def set_level(cls, level: LogLevel):
        """Set the global log level"""
        cls.initialize()
        # TODO: Call C library function
        pass

    @classmethod
    def get_level(cls) -> LogLevel:
        """Get the global log level"""
        cls.initialize()
        # TODO: Call C library function
        return LogLevel.INFO

    @classmethod
    def trace(cls, category: str, message: str):
        """Log a trace message"""
        cls._log(LogLevel.TRACE, category, message)

    @classmethod
    def debug(cls, category: str, message: str):
        """Log a debug message"""
        cls._log(LogLevel.DEBUG, category, message)

    @classmethod
    def info(cls, category: str, message: str):
        """Log an info message"""
        cls._log(LogLevel.INFO, category, message)

    @classmethod
    def warning(cls, category: str, message: str):
        """Log a warning message"""
        cls._log(LogLevel.WARNING, category, message)

    @classmethod
    def error(cls, category: str, message: str):
        """Log an error message"""
        cls._log(LogLevel.ERROR, category, message)

    @classmethod
    def critical(cls, category: str, message: str):
        """Log a critical message"""
        cls._log(LogLevel.CRITICAL, category, message)

    @classmethod
    def _log(cls, level: LogLevel, category: str, message: str):
        """Internal logging method"""
        cls.initialize()
        # TODO: Call C library telemetry_log_write()
        # For now, use Python logging as fallback
        import logging
        py_logger = logging.getLogger(category)

        level_map = {
            LogLevel.TRACE: logging.DEBUG,
            LogLevel.DEBUG: logging.DEBUG,
            LogLevel.INFO: logging.INFO,
            LogLevel.WARNING: logging.WARNING,
            LogLevel.ERROR: logging.ERROR,
            LogLevel.CRITICAL: logging.CRITICAL,
        }

        py_logger.log(level_map.get(level, logging.INFO), message)

    @classmethod
    def add_handler(cls, handler: LogHandler):
        """Add a log handler"""
        cls._handlers.append(handler)
        handler.register()

    @classmethod
    def remove_handler(cls, handler: LogHandler):
        """Remove a log handler"""
        if handler in cls._handlers:
            handler.unregister()
            cls._handlers.remove(handler)

    @classmethod
    def flush(cls):
        """Flush all pending log entries"""
        cls.initialize()
        # TODO: Call C library telemetry_log_flush()
        pass


class Metric:
    """Base class for all metrics"""

    def __init__(self, name: str, description: str = "", unit: str = ""):
        self.name = name
        self.description = description
        self.unit = unit
        self.labels: Dict[str, str] = {}
        self._metric = None

    def add_label(self, key: str, value: str):
        """Add a label to this metric"""
        self.labels[key] = value
        # TODO: Call C library function

    def register(self):
        """Register this metric with the registry"""
        # TODO: Call C library function
        pass

    def unregister(self):
        """Unregister this metric from the registry"""
        # TODO: Call C library function
        pass


class Counter(Metric):
    """Counter metric - monotonically increasing value"""

    def __init__(self, name: str, description: str = "", unit: str = ""):
        super().__init__(name, description, unit)
        self._value = 0

    def increment(self, value: int = 1):
        """Increment the counter"""
        # TODO: Call C library function
        self._value += value

    def get(self) -> int:
        """Get the current counter value"""
        # TODO: Call C library function
        return self._value

    def reset(self):
        """Reset the counter to zero"""
        # TODO: Call C library function
        self._value = 0


class Gauge(Metric):
    """Gauge metric - value that can go up and down"""

    def __init__(self, name: str, description: str = "", unit: str = ""):
        super().__init__(name, description, unit)
        self._value = 0.0

    def set(self, value: float):
        """Set the gauge value"""
        # TODO: Call C library function
        self._value = value

    def increment(self, value: float = 1.0):
        """Increment the gauge"""
        # TODO: Call C library function
        self._value += value

    def decrement(self, value: float = 1.0):
        """Decrement the gauge"""
        # TODO: Call C library function
        self._value -= value

    def get(self) -> float:
        """Get the current gauge value"""
        # TODO: Call C library function
        return self._value


class Histogram(Metric):
    """Histogram metric - statistical distribution"""

    def __init__(self, name: str, description: str = "", unit: str = "",
                 buckets: Optional[List[float]] = None):
        super().__init__(name, description, unit)
        self.buckets = buckets or [0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 5.0, 10.0]
        self._observations = []

    def observe(self, value: float):
        """Observe a value"""
        # TODO: Call C library function
        self._observations.append(value)

    def get_stats(self) -> Dict[str, Union[int, float]]:
        """Get histogram statistics"""
        # TODO: Call C library function
        if not self._observations:
            return {"count": 0, "sum": 0.0, "min": 0.0, "max": 0.0}

        return {
            "count": len(self._observations),
            "sum": sum(self._observations),
            "min": min(self._observations),
            "max": max(self._observations),
        }

    def reset(self):
        """Reset the histogram"""
        # TODO: Call C library function
        self._observations.clear()


class MetricRegistry:
    """Metric registry for managing metrics"""

    _initialized = False
    _metrics: Dict[str, Metric] = {}

    @classmethod
    def initialize(cls):
        """Initialize the metrics system"""
        if not cls._initialized:
            # TODO: Call C library telemetry_metrics_initialize()
            cls._initialized = True

    @classmethod
    def shutdown(cls):
        """Shutdown the metrics system"""
        if cls._initialized:
            # TODO: Call C library telemetry_metrics_shutdown()
            cls._initialized = False

    @classmethod
    def register(cls, metric: Metric):
        """Register a metric"""
        cls.initialize()
        cls._metrics[metric.name] = metric
        metric.register()

    @classmethod
    def unregister(cls, metric: Metric):
        """Unregister a metric"""
        if metric.name in cls._metrics:
            metric.unregister()
            del cls._metrics[metric.name]

    @classmethod
    def get(cls, name: str) -> Optional[Metric]:
        """Get a metric by name"""
        return cls._metrics.get(name)

    @classmethod
    def get_all(cls) -> List[Metric]:
        """Get all registered metrics"""
        return list(cls._metrics.values())

    @classmethod
    def export_all(cls, format_type: MetricExportFormat = MetricExportFormat.PROMETHEUS) -> str:
        """Export all metrics to string"""
        cls.initialize()
        # TODO: Call C library function

        # Fallback implementation
        output = []
        for metric in cls._metrics.values():
            if isinstance(metric, Counter):
                output.append(f"# TYPE {metric.name} counter")
                output.append(f"{metric.name} {metric.get()}")
            elif isinstance(metric, Gauge):
                output.append(f"# TYPE {metric.name} gauge")
                output.append(f"{metric.name} {metric.get()}")
            elif isinstance(metric, Histogram):
                stats = metric.get_stats()
                output.append(f"# TYPE {metric.name} histogram")
                output.append(f"{metric.name}_count {stats['count']}")
                output.append(f"{metric.name}_sum {stats['sum']}")

        return "\n".join(output)

    @classmethod
    def reset_all(cls):
        """Reset all metrics"""
        cls.initialize()
        # TODO: Call C library function
        for metric in cls._metrics.values():
            if hasattr(metric, 'reset'):
                metric.reset()


class MetricExporter:
    """Metric exporter for pushing metrics to external systems"""

    def __init__(self, format_type: MetricExportFormat, endpoint: str, push_interval: int = 0):
        self.format_type = format_type
        self.endpoint = endpoint
        self.push_interval = push_interval
        self._exporter = None

    def register(self):
        """Register this exporter"""
        # TODO: Call C library function
        pass

    def unregister(self):
        """Unregister this exporter"""
        # TODO: Call C library function
        pass

    def export(self) -> str:
        """Export metrics using this exporter"""
        # TODO: Call C library function
        return MetricRegistry.export_all(self.format_type)


# Convenience decorators
def timed(metric_name: str):
    """Decorator to time function execution and record in histogram"""
    import time

    def decorator(func):
        def wrapper(*args, **kwargs):
            histogram = MetricRegistry.get(metric_name)
            if histogram is None:
                histogram = Histogram(metric_name, f"Execution time of {func.__name__}", "seconds")
                MetricRegistry.register(histogram)

            start = time.time()
            try:
                return func(*args, **kwargs)
            finally:
                duration = time.time() - start
                histogram.observe(duration)

        return wrapper
    return decorator


def counted(metric_name: str):
    """Decorator to count function calls"""
    def decorator(func):
        def wrapper(*args, **kwargs):
            counter = MetricRegistry.get(metric_name)
            if counter is None:
                counter = Counter(metric_name, f"Call count of {func.__name__}", "calls")
                MetricRegistry.register(counter)

            counter.increment()
            return func(*args, **kwargs)

        return wrapper
    return decorator


# Module-level convenience functions
def get_logger(category: str) -> 'CategoryLogger':
    """Get a logger for a specific category"""
    return CategoryLogger(category)


class CategoryLogger:
    """Logger scoped to a specific category"""

    def __init__(self, category: str):
        self.category = category

    def trace(self, message: str):
        Logger.trace(self.category, message)

    def debug(self, message: str):
        Logger.debug(self.category, message)

    def info(self, message: str):
        Logger.info(self.category, message)

    def warning(self, message: str):
        Logger.warning(self.category, message)

    def error(self, message: str):
        Logger.error(self.category, message)

    def critical(self, message: str):
        Logger.critical(self.category, message)


# Initialize on import
Logger.initialize()
MetricRegistry.initialize()


__all__ = [
    'LogLevel', 'LogFormat', 'HandlerType', 'RotationPolicy',
    'MetricType', 'MetricExportFormat', 'TelemetryError',
    'Logger', 'LogHandler', 'LogFormatter',
    'Metric', 'Counter', 'Gauge', 'Histogram',
    'MetricRegistry', 'MetricExporter',
    'get_logger', 'CategoryLogger',
    'timed', 'counted'
]
