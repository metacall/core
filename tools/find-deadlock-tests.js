#!/usr/bin/env node
'use strict';

/* This script gets the output of ctest and returns
 * the non-finished tests or the failed ones, useful
 * for detecting deadlocks. Use it like:
 * $ node find-deadlock-tests.js ctest_output_file.txt
 */

const { readFileSync } = require('fs');

if (!process.argv[2]) {
    console.error('No file specified');
    process.exit(1);
}

const file = readFileSync(process.argv[2]);

if (!file) {
    console.error('Failed to read the file');
    process.exit(2);
}

const tests = file.toString().split('\n').reduce((tests, line) => {
    const splitted = line.split(' ').filter(element => element);
    if (splitted[0] === 'Start') {
        tests.start.push(splitted[2]);
    } else if (splitted[5] === 'Passed') {
        tests.finish.push(splitted[3]);
    }
    return tests;
}, { start: [], finish: [] });

const deadlock = tests.start.filter(x => !tests.finish.includes(x));

console.log(deadlock);
