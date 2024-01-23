const command_parser_map = {};

function command_parse(cmd) {
	original_command = cmd.toString();
	current = cmd.indexOf(' ');
	key = cmd.substring(0, current);
	cmd = cmd.substring(++current);

	const skip_whitespaces = (str, position) => {
		while (str[position] === ' ') {
			++position;
		}

		return position;
	}

	const command = command_parser_map[key];
	const tokens = [];

	for (let param_count = 0; param_count < command.length; ++param_count) {
		current = skip_whitespaces(current);
		next = cmd.indexOf(' ')
		param = cmd.substring(current, next);

		switch (command.types[param_count]) {
			case 'METACALL_STRING': {
				if (!command.regexes[param_count].test(param)) {
					return null;
				}
				tokens.push(param);
				break;
			}

			case 'METACALL_ARRAY': {
				const array = [];
				if (!command.regexes[param_count].test(param)) {
					return null;
				}
				array.push(param);

				do {
					
				}
				break;
			}

			default:
				console.error(`Failed to parse command '${original_command}', type ${command.types[param_count]} is not implemented`)
				return null;
		}

		cmd = cmd.substring(next);
	}

	return tokens;
}

module.exports = {
	command_register: (cmd, regexes) => {
		command_parser_map[cmd] = regexes.map(re => new RegExp(re));
	},
	command_complete: () => {},
	command_parse,
};
