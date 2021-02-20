function hello(name) {
    console.log('Argument: ', name)
    console.log(name)
    if (name) return `Hello, ${name}!`
    else return "Hello!"
}

function increment(n) {
    return n + 1
}

function env() {
    console.log(process.env);
}

function sumListJs(arr) {
    return arr.reduce((acc, val) => acc + val, 0);
}

async function sleep(ms) {
    return new Promise((resolve) => {
        setTimeout(resolve, ms);
    });
}

module.exports = { hello, env, increment, sumListJs, sleep };
