#!/usr/bin/env node

/* This has been ripped off from NPM and adapted to be callable instead of invoked by exec */

function package_manager(args) {
	// windows: running "npm blah" in this folder will invoke WSH, not node.
	/* global WScript */
	if (typeof WScript !== 'undefined') {
		WScript.echo(
			'npm does not work when run\n' +
				'with the Windows Scripting Host\n\n' +
				"'cd' to a different directory,\n" +
				"or type 'npm.cmd <args>',\n" +
				"or type 'node npm <args>'."
		)
		WScript.quit(1)
		return
	}

	var unsupported = require('npm/lib/utils/unsupported.js')
	unsupported.checkForBrokenNode()

	var log = require('npm/node_modules/npmlog')
	log.pause() // will be unpaused when config is loaded.
	log.info('it worked if it ends with', 'ok')

	unsupported.checkForUnsupportedNode()

	var path = require('path')
	var npm = require('npm/lib/npm.js')
	var npmconf = require('npm/lib/config/core.js')
	var errorHandler = require('npm/lib/utils/error-handler.js')

	var configDefs = npmconf.defs
	var shorthands = configDefs.shorthands
	var types = configDefs.types
	var nopt = require('npm/node_modules/nopt')

	// Overwrite process args
	process.argv = [ 'node', 'npm', ...args ];

	log.verbose('cli', process.argv)

	var conf = nopt(types, shorthands)
	npm.argv = conf.argv.remain
	if (npm.deref(npm.argv[0])) npm.command = npm.argv.shift()
	else conf.usage = true

	if (conf.version) {
		console.log(npm.version)
		return errorHandler.exit(0)
	}

	if (conf.versions) {
		npm.command = 'version'
		conf.usage = false
		npm.argv = []
	}

	log.info('using', 'npm@%s', npm.version)
	log.info('using', 'node@%s', process.version)

	process.on('uncaughtException', errorHandler)

	if (conf.usage && npm.command !== 'help') {
		npm.argv.unshift(npm.command)
		npm.command = 'help'
	}

	var isGlobalNpmUpdate = conf.global && ['install', 'update'].includes(npm.command) && npm.argv.includes('npm')

	// now actually fire up npm and run the command.
	// this is how to use npm programmatically:
	conf._exit = true
	npm.load(conf, function (er) {
		if (er) return errorHandler(er)
		if (
			!isGlobalNpmUpdate &&
			npm.config.get('update-notifier') &&
			!unsupported.checkVersion(process.version).unsupported
		) {
			const pkg = require('npm/package.json')
			let notifier = require('npm/node_modules/update-notifier')({pkg})
			const isCI = require('npm/node_modules/ci-info').isCI
			if (
				notifier.update &&
				notifier.update.latest !== pkg.version &&
				!isCI
			) {
				const color = require('ansicolors')
				const useColor = npm.config.get('color')
				const useUnicode = npm.config.get('unicode')
				const old = notifier.update.current
				const latest = notifier.update.latest
				let type = notifier.update.type
				if (useColor) {
					switch (type) {
						case 'major':
							type = color.red(type)
							break
						case 'minor':
							type = color.yellow(type)
							break
						case 'patch':
							type = color.green(type)
							break
					}
				}
				const changelog = `https://github.com/npm/cli/releases/tag/v${latest}`
				notifier.notify({
					message: `New ${type} version of ${pkg.name} available! ${
						useColor ? color.red(old) : old
					} ${useUnicode ? '→' : '->'} ${
						useColor ? color.green(latest) : latest
					}\n` +
					`${
						useColor ? color.yellow('Changelog:') : 'Changelog:'
					} ${
						useColor ? color.cyan(changelog) : changelog
					}\n` +
					`Run ${
						useColor
							? color.green(`npm install -g ${pkg.name}`)
							: `npm i -g ${pkg.name}`
					} to update!`
				})
			}
		}
		npm.commands[npm.command](npm.argv, function (err) {
			// https://genius.com/Lin-manuel-miranda-your-obedient-servant-lyrics
			if (
				!err &&
				npm.config.get('ham-it-up') &&
				!npm.config.get('json') &&
				!npm.config.get('parseable') &&
				npm.command !== 'completion'
			) {
				console.error(
					`\n ${
						npm.config.get('unicode') ? '🎵 ' : ''
					} I Have the Honour to Be Your Obedient Servant,${
						npm.config.get('unicode') ? '🎵 ' : ''
					} ~ npm ${
						npm.config.get('unicode') ? '📜🖋 ' : ''
					}\n`
				)
			}
			errorHandler.apply(this, arguments)
		})
	})
}

module.exports = {
	package_manager,
};
