export function this_will_be_loaded_without_tsconfig(a: () => void): () => void {
	return a;
}
