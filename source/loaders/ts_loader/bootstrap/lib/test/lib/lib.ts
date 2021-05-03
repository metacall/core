export function this_will_use_some_random_lib(a: number[]): number[] {
	const entries = new Map([
		['foo', 'bar'],
		['A', 'B']
	]);
	const obj = Object.fromEntries(entries);
	console.log(obj);
	return { ...obj, ...a };
}

console.log(this_will_use_some_random_lib([34]));
