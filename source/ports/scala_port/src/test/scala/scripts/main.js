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

module.exports = { hello, env, increment }
