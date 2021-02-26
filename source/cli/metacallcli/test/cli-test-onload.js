const { metacall_load_from_memory, metacall } = require('metacall');

metacall_load_from_memory('py', 'def sum(a, b):\n\treturn a + b');
metacall_load_from_memory('rb', 'def mult(a, b)\n\ta * b\nend');

metacall('sum', 3, 4) !== 7 && process.exit(1);
metacall('mult', 3, 4) !== 12 && process.exit(1);
