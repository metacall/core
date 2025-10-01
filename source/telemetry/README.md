# MetaCall Telemetry Library

A comprehensive logging and metrics telemetry system for the MetaCall polyglot runtime.

## Features

### Logging System
- **Multiple Log Levels**: TRACE, DEBUG, INFO, WARNING, ERROR, CRITICAL
- **Flexible Handlers**: Console, File, Syslog, Network, and Custom handlers
- **Multiple Output Formats**: Plain text, JSON, XML, and ANSI colored output
- **Log Rotation**: Support for size-based, time-based, daily, weekly, and monthly rotation
- **Filtering**: Category-based and level-based filtering with custom callbacks
- **Thread-Safe**: Built with concurrent access in mind
- **Async Logging**: Optional asynchronous logging for high-performance scenarios

### Metrics System
- **Metric Types**:
  - **Counters**: Monotonically increasing values (e.g., request counts, error counts)
  - **Gauges**: Values that can increase or decrease (e.g., memory usage, active connections)
  - **Histograms**: Statistical distributions with configurable buckets
- **Labels**: Multi-dimensional metrics with key-value labels
- **Export Formats**: Prometheus, JSON, StatsD, InfluxDB, and Graphite
- **Metric Registry**: Centralized metric management
- **Aggregation**: Built-in support for SUM, AVG, MIN, MAX, COUNT, and RATE
- **Auto-Export**: Configurable exporters with push intervals

### Language Bindings
- **C**: Native implementation with full API access
- **Python**: Pythonic interface with decorators and context managers
- **JavaScript/Node.js**: Promise-based async API
- **Java**: Thread-safe implementation with builder patterns

## Architecture

```
telemetry/
├── include/telemetry/          # Public headers
│   ├── telemetry_api.h         # API export definitions
│   ├── telemetry_log.h         # Logging system API
│   ├── telemetry_metrics.h     # Metrics system API
│   └── telemetry_config.h      # Configuration management
├── source/                     # Implementation
│   ├── telemetry_log.c         # Logging implementation
│   └── telemetry_metrics.c     # Metrics implementation
├── bindings/                   # Language bindings
│   ├── python/                 # Python bindings
│   ├── javascript/             # JavaScript/Node.js bindings
│   └── java/                   # Java bindings
└── test/                       # Test suite
    ├── test_log.c              # C logging tests
    ├── test_metrics.c          # C metrics tests
    ├── test_telemetry.py       # Python tests
    └── test_telemetry.js       # JavaScript tests
```

## Quick Start

### C API

```c
#include <telemetry/telemetry_log.h>
#include <telemetry/telemetry_metrics.h>

int main() {
    // Initialize telemetry
    telemetry_log_initialize();
    telemetry_metrics_initialize();

    // Create and register a console handler
    telemetry_log_handler handler = telemetry_log_handler_create(
        TELEMETRY_LOG_HANDLER_CONSOLE,
        TELEMETRY_LOG_LEVEL_INFO
    );
    telemetry_log_handler_register(handler);

    // Log messages
    TELEMETRY_LOG_INFO("app", "Application started");
    TELEMETRY_LOG_DEBUG("app", "Processing request");
    TELEMETRY_LOG_ERROR("app", "Error occurred");

    // Create and use metrics
    telemetry_metric counter = telemetry_metric_counter_create(
        "requests_total",
        "Total requests",
        "requests"
    );
    telemetry_metric_register(counter);
    telemetry_metric_counter_increment(counter, 1);

    // Cleanup
    telemetry_log_shutdown();
    telemetry_metrics_shutdown();

    return 0;
}
```

### Python API

```python
from telemetry import Logger, Counter, MetricRegistry

# Initialize
Logger.initialize()
MetricRegistry.initialize()

# Logging
Logger.info("app", "Application started")
Logger.error("app", "An error occurred")

# Category logger
logger = Logger.get_logger("myapp")
logger.info("Processing request")

# Metrics
counter = Counter("requests_total", "Total requests", "requests")
MetricRegistry.register(counter)
counter.increment()

# Decorators
@timed("function_duration")
def process_request():
    # Your code here
    pass

@counted("function_calls")
def api_handler():
    # Your code here
    pass
```

### JavaScript API

```javascript
const { Logger, LogLevel, Counter, MetricRegistry } = require('telemetry');

// Initialize
Logger.initialize();
MetricRegistry.initialize();

// Logging
Logger.setLevel(LogLevel.DEBUG);
Logger.info("app", "Application started");
Logger.error("app", "An error occurred");

// Category logger
const logger = getLogger("myapp");
logger.info("Processing request");

// Metrics
const counter = new Counter("requests_total", "Total requests", "requests");
MetricRegistry.register(counter);
counter.increment();

// Export metrics
const prometheusData = MetricRegistry.exportAll(MetricExportFormat.PROMETHEUS);
console.log(prometheusData);
```

### Java API

```java
import com.metacall.telemetry.*;

public class Example {
    public static void main(String[] args) {
        // Initialize
        Logger.initialize();
        MetricRegistry.initialize();

        // Logging
        Logger.setLevel(LogLevel.DEBUG);
        Logger.info("app", "Application started");
        Logger.error("app", "An error occurred");

        // Category logger
        CategoryLogger logger = Logger.getLogger("myapp");
        logger.info("Processing request");

        // Metrics
        Counter counter = new Counter("requests_total", "Total requests", "requests");
        MetricRegistry.register(counter);
        counter.increment();

        // Export metrics
        String prometheusData = MetricRegistry.exportAll(MetricExportFormat.PROMETHEUS);
        System.out.println(prometheusData);

        // Cleanup
        Logger.shutdown();
        MetricRegistry.shutdown();
    }
}
```

## Configuration

### JSON Configuration

```json
{
  "enabled": true,
  "log_level": "INFO",
  "async_logging": false,
  "handlers": [
    {
      "type": "CONSOLE",
      "min_level": "INFO",
      "format": "COLORED",
      "enabled": true
    },
    {
      "type": "FILE",
      "min_level": "DEBUG",
      "format": "JSON",
      "file_path": "/var/log/metacall/app.log",
      "rotation": "DAILY",
      "max_file_size": 10485760,
      "enabled": true
    }
  ],
  "metrics_enabled": true,
  "exporters": [
    {
      "format": "PROMETHEUS",
      "endpoint": "/tmp/metrics.txt",
      "push_interval": 60,
      "enabled": true
    }
  ]
}
```

### Loading Configuration

```c
// Load from file
telemetry_config config = telemetry_config_load_from_file("config.json");
telemetry_config_apply(config);

// Load from string
const char *json_config = "{...}";
telemetry_config config = telemetry_config_load_from_string(json_config);
telemetry_config_apply(config);
```

## Log Formats

### Plain Text
```
[2025-10-01 12:34:56] [INFO] [app] Application started (main.c:42 in main)
```

### JSON
```json
{"timestamp":"2025-10-01 12:34:56","level":"INFO","category":"app","message":"Application started","file":"main.c","line":42,"function":"main"}
```

### XML
```xml
<log><timestamp>2025-10-01 12:34:56</timestamp><level>INFO</level><category>app</category><message>Application started</message><location><file>main.c</file><line>42</line><function>main</function></location></log>
```

### Colored (ANSI)
Automatically colors output based on log level (errors in red, warnings in yellow, etc.)

## Metric Export Formats

### Prometheus
```
# HELP requests_total Total HTTP requests
# TYPE requests_total counter
requests_total{method="GET",status="200"} 1234

# HELP response_time_seconds HTTP response time
# TYPE response_time_seconds histogram
response_time_seconds_bucket{le="0.1"} 100
response_time_seconds_bucket{le="0.5"} 450
response_time_seconds_bucket{le="1.0"} 890
response_time_seconds_sum 423.5
response_time_seconds_count 1000
```

### JSON
```json
[
  {
    "name": "requests_total",
    "type": "counter",
    "description": "Total HTTP requests",
    "unit": "requests",
    "value": 1234,
    "labels": {
      "method": "GET",
      "status": "200"
    }
  }
]
```

### InfluxDB Line Protocol
```
requests_total,method=GET,status=200 value=1234i
response_time,endpoint=/api value=0.142
```

### StatsD
```
requests_total:1234|c
response_time:142|ms
memory_usage:1024|g
```

### Graphite
```
requests_total 1234 1633089296
response_time.p50 0.142 1633089296
memory_usage 1024 1633089296
```

## Performance Considerations

1. **Async Logging**: Enable for high-throughput scenarios
   ```c
   telemetry_log_set_async(1);
   ```

2. **Log Level Filtering**: Set appropriate minimum levels to reduce overhead
   ```c
   telemetry_log_set_level(TELEMETRY_LOG_LEVEL_WARNING);
   ```

3. **Metric Sampling**: For high-cardinality metrics, consider sampling
   ```c
   if (rand() % 100 < sample_rate) {
       telemetry_metric_counter_increment(counter, 1);
   }
   ```

4. **Label Cardinality**: Avoid high-cardinality labels (e.g., user IDs, timestamps)
   - Good: method, status_code, endpoint
   - Bad: user_id, request_id, timestamp

## Thread Safety

All telemetry operations are thread-safe by design:
- Log handlers use internal locking
- Metrics use atomic operations where possible
- Registry operations are protected by mutexes

## Building

### CMake
```bash
mkdir build
cd build
cmake ..
make
make test
```

### Integration with MetaCall
The telemetry library is automatically built as part of the MetaCall core build process.

## Testing

### Running C Tests
```bash
./test_log
./test_metrics
```

### Running Python Tests
```bash
python3 test_telemetry.py
```

### Running JavaScript Tests
```bash
node test_telemetry.js
```

### Running Java Tests
```bash
javac com/metacall/telemetry/*.java
java -cp . com.metacall.telemetry.TestTelemetry
```

## Best Practices

1. **Use Structured Logging**: Include context in log messages
   ```c
   TELEMETRY_LOG_INFO("http", "Request processed: method=%s path=%s status=%d duration=%dms",
       method, path, status, duration);
   ```

2. **Create Category Loggers**: Organize logs by component
   ```python
   logger = get_logger("database")
   logger.info("Connected to database")
   ```

3. **Use Metric Labels Wisely**: Keep cardinality low
   ```java
   counter.addLabel("method", "GET");
   counter.addLabel("status", "200");
   // Avoid: counter.addLabel("user_id", userId);
   ```

4. **Monitor Export Performance**: Large metric sets may need optimization
   ```c
   // Export periodically rather than on every request
   telemetry_metric_exporter_create(TELEMETRY_METRIC_EXPORT_PROMETHEUS,
       "/metrics", 60); // 60 second interval
   ```

5. **Clean Up Resources**: Always shutdown properly
   ```c
   telemetry_log_flush();
   telemetry_log_shutdown();
   telemetry_metrics_shutdown();
   ```

## Troubleshooting

### Logs Not Appearing
- Check log level settings
- Verify handler is registered
- Ensure handler is enabled

### Metrics Not Exporting
- Verify exporter is registered
- Check file permissions for file exporters
- Ensure metrics are registered before export

### Performance Issues
- Enable async logging
- Reduce log level in production
- Sample high-frequency metrics
- Monitor label cardinality

## Contributing

Contributions are welcome! Please ensure:
- All tests pass
- Code follows project style
- Documentation is updated
- Thread safety is maintained

## License

GNU Lesser General Public License v3.0 or later

Copyright (C) 2025 MetaCall Inc., Dhiren Mhatre

## Support

- Documentation: https://github.com/metacall/core/docs
- Issues: https://github.com/metacall/core/issues
- Community: https://t.me/metacall
- Discord: https://discord.gg/upwP4mwJWa
