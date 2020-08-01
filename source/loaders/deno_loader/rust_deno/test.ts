type Foo<T> = { foo: Foo<T> }

console.log('poopballs');

Deno.openPlugin('../rust_deno_plugin/target/debug/librust_deno_plugin.so')

const core = (Deno as any).core;

const { metacall } = core.ops();

const result: Uint8Array = core.dispatch(metacall, new Uint8Array([]));

const stringified = [ ...result].map(x => String.fromCharCode(x)).join('');

console.log(stringified);
