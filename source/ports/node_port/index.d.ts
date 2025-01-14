declare module 'metacall' {
	export function metacall(name: string, ...args: any): any;
	export function metacallfms(name: string, buffer: string): any;
	export function metacall_execution_path(tag: string, path: string): number;
	export function metacall_load_from_file(tag: string, paths: string[]): number;
	export function metacall_load_from_file_export(tag: string, paths: string[]): any;
	export function metacall_load_from_memory(tag: string, code: string): number;
	export function metacall_load_from_memory_export(tag: string, code: string): any;
	export function metacall_load_from_package(tag: string, pkg: string): number;
	export function metacall_load_from_package_export(tag: string, pkg: string): any;
	export function metacall_load_from_configuration(path: string): number;
	export function metacall_load_from_configuration_export(path: string): any;
	export function metacall_inspect(): any;
	export function metacall_handle(tag: string, name: string): any;
	export function metacall_logs(): void;
}
