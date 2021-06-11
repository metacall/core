declare module 'metacall' {
	export function metacall(name: string, ...args: any): any;
	export function metacall_load_from_file(tag: string, paths: string[]): number;
	export function metacall_load_from_file_export(tag: string, paths: string[]): any;
	export function metacall_load_from_memory(tag: string, code: string): number;
	export function metacall_load_from_memory_export(tag: string, code: string): any;
	export function metacall_inspect(): any;
	export function metacall_handle(tag: string, name: string): any;
	export function metacall_logs(): void;
}
