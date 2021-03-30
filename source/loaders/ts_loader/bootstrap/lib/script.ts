#!/usr/bin/env node
'use strict';

export function test_array(a: number[]): number[] {
	return a;
}

export function test_implicit_object({ a }) {
	return a;
}

export function test_record(a: Record<string, Int16Array>): Record<string, Int16Array> {
	return a;
}

export function test_function(a: () => void): () => void {
	return a;
}
