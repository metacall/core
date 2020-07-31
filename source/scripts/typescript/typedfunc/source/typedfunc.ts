'use strict';

export function typed_sum(left: number, rigth: number): number {
	return left + rigth;
}

export async function typed_sum_async(left: number, rigth: number): number {
	return left + rigth;
}

export function build_name(first: string, last = 'Smith') {
	return `${first} ${last}`;
}

export function object_pattern_ts({ asd }) {
	return asd;
}
