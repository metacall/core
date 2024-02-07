const command_parser_map = {};

function command_parse(cmd) {
	const command_str = cmd.toString();
	let current_position = command_str.indexOf(' ');

	if (current_position === -1) {
		current_position = command_str.length;
	}

	const key = command_str.substring(0, current_position);

	if (key === '') {
		return [];
	}

	const command = command_parser_map[key];

	if (!command) {
		throw new Error(`Command '${key}' not valid, use 'help' for more information`);
	}

	const tokens = [];

	const skip_whitespaces = (str, position) => {
		while (str[position] === ' ') {
			++position;
		}

		return position;
	}

	for (let param_count = 0; param_count < command.regexes.length; ++param_count) {
		current_position = skip_whitespaces(command_str, current_position);
		const param = command_str.substring(current_position);
		const current_type = command.types[param_count];
		const current_regex = command.regexes[param_count];

		switch (current_type) {
			case 'METACALL_STRING': {
				if (!current_regex.test(param)) {
					throw new Error(`Failed to parse the command: '${param}' of type ${current_type}, the expected regex was: ${current_regex.toString()}`)
				}
				const next_position = current_regex.lastIndex;
				tokens.push(param.substring(0, next_position));
				current_position += next_position;
				break;
			}
			case 'METACALL_ARRAY': {
				const values = []

				/* Array will iterate meanwhile the regex matches or there is characters left */
				while (current_position < command_str.length) {
					const subparam = command_str.substring(current_position);
					if (!current_regex.test(subparam)) {
						break;
					}

					/* Obtain the next position and push the current value, then skip whitespaces */
					const next_position = current_regex.lastIndex;
					values.push(subparam.substring(0, next_position));
					current_position = skip_whitespaces(command_str, current_position + next_position);

					/* Reset last index in order to test the next value */
					current_regex.lastIndex = 0;
				}
				tokens.push(values);
				break;
			}

			default: {
				throw new Error(`Failed to parse command '${original_command}', type ${current_type} is not implemented`);
			}
		}

		/* Reset the regex for the next command */
		current_regex.lastIndex = 0;
	}

	return [key, ...tokens]
}

const command_register = (cmd, regexes, types) => {
	if (regexes.length !== types.length) {
		throw new Error('Number of Regexes and Types do not match')
	}
	command_parser_map[cmd] = {
		regexes: regexes.map(re => new RegExp(re, 'g')),
		types
	}
}

module.exports = {
	command_register,
	command_register_map: (map) => {
		Object.keys(map).forEach(key => command_register(key, map[key].regexes, map[key].types));
	},
	command_completer: () => Object.keys(command_parser_map),
	command_parse,
};
