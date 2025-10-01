/*
 *	Telemetry Library by MetaCall Inc.
 *	Test suite for telemetry logging system.
 *
 *	Copyright (C) 2025 MetaCall Inc., Dhiren Mhatre
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 */

#include <telemetry/telemetry_log.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Test initialization and shutdown */
void test_log_init_shutdown(void)
{
	printf("Testing log initialization and shutdown...\n");

	int result = telemetry_log_initialize();
	assert(result == 0);

	result = telemetry_log_shutdown();
	assert(result == 0);

	printf("✓ Log initialization and shutdown test passed\n");
}

/* Test log level management */
void test_log_levels(void)
{
	printf("Testing log levels...\n");

	telemetry_log_initialize();

	telemetry_log_set_level(TELEMETRY_LOG_LEVEL_DEBUG);
	assert(telemetry_log_get_level() == TELEMETRY_LOG_LEVEL_DEBUG);

	telemetry_log_set_level(TELEMETRY_LOG_LEVEL_ERROR);
	assert(telemetry_log_get_level() == TELEMETRY_LOG_LEVEL_ERROR);

	telemetry_log_shutdown();

	printf("✓ Log levels test passed\n");
}

/* Test log level string conversion */
void test_log_level_strings(void)
{
	printf("Testing log level string conversion...\n");

	telemetry_log_initialize();

	assert(strcmp(telemetry_log_level_to_string(TELEMETRY_LOG_LEVEL_TRACE), "TRACE") == 0);
	assert(strcmp(telemetry_log_level_to_string(TELEMETRY_LOG_LEVEL_DEBUG), "DEBUG") == 0);
	assert(strcmp(telemetry_log_level_to_string(TELEMETRY_LOG_LEVEL_INFO), "INFO") == 0);
	assert(strcmp(telemetry_log_level_to_string(TELEMETRY_LOG_LEVEL_WARNING), "WARNING") == 0);
	assert(strcmp(telemetry_log_level_to_string(TELEMETRY_LOG_LEVEL_ERROR), "ERROR") == 0);
	assert(strcmp(telemetry_log_level_to_string(TELEMETRY_LOG_LEVEL_CRITICAL), "CRITICAL") == 0);

	assert(telemetry_log_level_from_string("TRACE") == TELEMETRY_LOG_LEVEL_TRACE);
	assert(telemetry_log_level_from_string("DEBUG") == TELEMETRY_LOG_LEVEL_DEBUG);
	assert(telemetry_log_level_from_string("INFO") == TELEMETRY_LOG_LEVEL_INFO);
	assert(telemetry_log_level_from_string("WARNING") == TELEMETRY_LOG_LEVEL_WARNING);
	assert(telemetry_log_level_from_string("ERROR") == TELEMETRY_LOG_LEVEL_ERROR);
	assert(telemetry_log_level_from_string("CRITICAL") == TELEMETRY_LOG_LEVEL_CRITICAL);

	telemetry_log_shutdown();

	printf("✓ Log level string conversion test passed\n");
}

/* Test handler creation and destruction */
void test_log_handlers(void)
{
	printf("Testing log handlers...\n");

	telemetry_log_initialize();

	telemetry_log_handler console_handler = telemetry_log_handler_create(
		TELEMETRY_LOG_HANDLER_CONSOLE,
		TELEMETRY_LOG_LEVEL_INFO);
	assert(console_handler != NULL);

	int result = telemetry_log_handler_register(console_handler);
	assert(result == 0);

	result = telemetry_log_handler_unregister(console_handler);
	assert(result == 0);

	telemetry_log_handler_destroy(console_handler);

	telemetry_log_shutdown();

	printf("✓ Log handlers test passed\n");
}

/* Test file handler configuration */
void test_file_handler(void)
{
	printf("Testing file handler...\n");

	telemetry_log_initialize();

	telemetry_log_handler file_handler = telemetry_log_handler_create(
		TELEMETRY_LOG_HANDLER_FILE,
		TELEMETRY_LOG_LEVEL_DEBUG);
	assert(file_handler != NULL);

	int result = telemetry_log_handler_configure_file(
		file_handler,
		"/tmp/test_telemetry.log",
		TELEMETRY_LOG_ROTATION_SIZE,
		1024 * 1024); /* 1MB */
	assert(result == 0);

	result = telemetry_log_handler_register(file_handler);
	assert(result == 0);

	telemetry_log_handler_unregister(file_handler);
	telemetry_log_handler_destroy(file_handler);

	telemetry_log_shutdown();

	printf("✓ File handler test passed\n");
}

/* Test formatter creation */
void test_log_formatters(void)
{
	printf("Testing log formatters...\n");

	telemetry_log_initialize();

	telemetry_log_formatter text_formatter = telemetry_log_formatter_create(TELEMETRY_LOG_FORMAT_TEXT);
	assert(text_formatter != NULL);

	int result = telemetry_log_formatter_configure(text_formatter, 1, 1, 1, 1, 0);
	assert(result == 0);

	telemetry_log_formatter_destroy(text_formatter);

	telemetry_log_formatter json_formatter = telemetry_log_formatter_create(TELEMETRY_LOG_FORMAT_JSON);
	assert(json_formatter != NULL);
	telemetry_log_formatter_destroy(json_formatter);

	telemetry_log_shutdown();

	printf("✓ Log formatters test passed\n");
}

/* Test filter creation */
void test_log_filters(void)
{
	printf("Testing log filters...\n");

	telemetry_log_initialize();

	telemetry_log_filter filter = telemetry_log_filter_create();
	assert(filter != NULL);

	int result = telemetry_log_filter_set_category(filter, "test.*");
	assert(result == 0);

	result = telemetry_log_filter_set_level(filter, TELEMETRY_LOG_LEVEL_WARNING);
	assert(result == 0);

	result = telemetry_log_filter_register(filter);
	assert(result == 0);

	result = telemetry_log_filter_unregister(filter);
	assert(result == 0);

	telemetry_log_filter_destroy(filter);

	telemetry_log_shutdown();

	printf("✓ Log filters test passed\n");
}

/* Test logging messages */
void test_log_messages(void)
{
	printf("Testing log messages...\n");

	telemetry_log_initialize();
	telemetry_log_set_level(TELEMETRY_LOG_LEVEL_TRACE);

	telemetry_log_handler console_handler = telemetry_log_handler_create(
		TELEMETRY_LOG_HANDLER_CONSOLE,
		TELEMETRY_LOG_LEVEL_TRACE);

	telemetry_log_formatter formatter = telemetry_log_formatter_create(TELEMETRY_LOG_FORMAT_TEXT);
	telemetry_log_handler_set_formatter(console_handler, formatter);
	telemetry_log_handler_register(console_handler);

	TELEMETRY_LOG_TRACE("test", "This is a trace message");
	TELEMETRY_LOG_DEBUG("test", "This is a debug message");
	TELEMETRY_LOG_INFO("test", "This is an info message");
	TELEMETRY_LOG_WARNING("test", "This is a warning message");
	TELEMETRY_LOG_ERROR("test", "This is an error message");
	TELEMETRY_LOG_CRITICAL("test", "This is a critical message");

	telemetry_log_flush();

	telemetry_log_handler_unregister(console_handler);
	telemetry_log_handler_destroy(console_handler);

	telemetry_log_shutdown();

	printf("✓ Log messages test passed\n");
}

/* Test colored output formatter */
void test_colored_formatter(void)
{
	printf("Testing colored formatter...\n");

	telemetry_log_initialize();

	telemetry_log_handler console_handler = telemetry_log_handler_create(
		TELEMETRY_LOG_HANDLER_CONSOLE,
		TELEMETRY_LOG_LEVEL_TRACE);

	telemetry_log_formatter colored_formatter = telemetry_log_formatter_create(TELEMETRY_LOG_FORMAT_COLORED);
	telemetry_log_handler_set_formatter(console_handler, colored_formatter);
	telemetry_log_handler_register(console_handler);

	TELEMETRY_LOG_INFO("test", "Testing colored output");
	TELEMETRY_LOG_ERROR("test", "Testing colored error output");

	telemetry_log_handler_unregister(console_handler);
	telemetry_log_handler_destroy(console_handler);

	telemetry_log_shutdown();

	printf("✓ Colored formatter test passed\n");
}

/* Test JSON formatter */
void test_json_formatter(void)
{
	printf("Testing JSON formatter...\n");

	telemetry_log_initialize();

	telemetry_log_handler console_handler = telemetry_log_handler_create(
		TELEMETRY_LOG_HANDLER_CONSOLE,
		TELEMETRY_LOG_LEVEL_INFO);

	telemetry_log_formatter json_formatter = telemetry_log_formatter_create(TELEMETRY_LOG_FORMAT_JSON);
	telemetry_log_handler_set_formatter(console_handler, json_formatter);
	telemetry_log_handler_register(console_handler);

	TELEMETRY_LOG_INFO("test", "Testing JSON output");

	telemetry_log_handler_unregister(console_handler);
	telemetry_log_handler_destroy(console_handler);

	telemetry_log_shutdown();

	printf("✓ JSON formatter test passed\n");
}

/* Test XML formatter */
void test_xml_formatter(void)
{
	printf("Testing XML formatter...\n");

	telemetry_log_initialize();

	telemetry_log_handler console_handler = telemetry_log_handler_create(
		TELEMETRY_LOG_HANDLER_CONSOLE,
		TELEMETRY_LOG_LEVEL_INFO);

	telemetry_log_formatter xml_formatter = telemetry_log_formatter_create(TELEMETRY_LOG_FORMAT_XML);
	telemetry_log_handler_set_formatter(console_handler, xml_formatter);
	telemetry_log_handler_register(console_handler);

	TELEMETRY_LOG_INFO("test", "Testing XML output");

	telemetry_log_handler_unregister(console_handler);
	telemetry_log_handler_destroy(console_handler);

	telemetry_log_shutdown();

	printf("✓ XML formatter test passed\n");
}

/* Main test runner */
int main(void)
{
	printf("=== Telemetry Logging System Tests ===\n\n");

	test_log_init_shutdown();
	test_log_levels();
	test_log_level_strings();
	test_log_handlers();
	test_file_handler();
	test_log_formatters();
	test_log_filters();
	test_log_messages();
	test_colored_formatter();
	test_json_formatter();
	test_xml_formatter();

	printf("\n=== All Tests Passed ===\n");

	return 0;
}
