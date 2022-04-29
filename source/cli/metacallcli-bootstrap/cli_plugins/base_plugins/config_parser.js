  /**
  * Plugins return from <plugin-name>_init() this JSON config
  * 
  * @json {
  *  "version": "",
  *  "author": "",
  *  "help": "",
  *  "plugin_type":"",
  *  "apis": [] //array of (names) of implemented functions
  *  "pos_args":boolean,
  *  "options": [
  *     {
  *       "name": "",
  *       "value_type": ""
  *       "help":""
  *     }
  *   ]
  * }
  * @value_type can be {number} | {string} | {boolean} | {null} | {stringlist}.
  * a flag is an option with a value type of {null}
  * @pos_args is the number of positional args (not flags or options) the plugin takes. eg. serve 4000 (positonal args = 1)
  * Their order in the array should correspond to the order in which they appear on the command line
  * }
  * */

function config_parser_init () {
  let init = Object.create(null)
  init.help = "Plugin configuration parser"
  init.plugin_type = "BASE"
  init.apis = [
    "config_parser_get_help_txt",
    "config_parser_get_type",
    "config_parser_get_apis",
    "config_parser_get_options",
    "config_parser_check_config",
    "config_parser_supports_pos_args"
  ]
  init.pos_args = false;

 return JSON.stringify(init)
}

/**NOOP function. **/
function config_parser_fini(argument) {
  // body...
  return 0
}

function config_parser_check_config (json_data) {
  let config
  try {
    config = JSON.parse(json_data)
  } catch (e) {
    return "JSON_ERROR";
  }
  let errors = []
  if (!config.plugin_type) {
    errors.push({
      message: "Plugin type not defined"
    })

  } else if (config.plugin_type === "BASE" || config.plugin_type === "COMMAND") {
    errors.push({
      message: "Invalid plugin type"
    })
  }

  if (!Array.isArray(config.apis) || config.apis.length <= 0) {
    errors.push({
      message: "APIS not defined"
    })
  }

  if (!config.help || (typeof config.help !== 'string' || !(config.help instanceof String)) ) {
    errors.push({
      message: "Help string not defined"
    })
  }

  if (errors.length == 0)
    return ""
  else
    return errors.join('\n') 
}

function config_parser_get_help_txt (json_data) {
  let config = JSON.parse(json_data)
    return config.help;
}

function config_parser_get_type (json_data) {
  let config = JSON.parse(json_data)
    return config.plugin_type
}

function config_parser_get_apis(json_data) {
  let config = JSON.parse(json_data)
  return config.apis.join()
}
/*
function config_parser_get_options(json_data) {
  // Todo
}
*/

function config_parser_supports_pos_args(json_data) {
  let config = JSON.parse(json_data)
  return config.pos_args
}

exports.config_parser_init = config_parser_init
exports.config_parser_fini = config_parser_fini
exports.config_parser_get_type = config_parser_get_type
exports.config_parser_get_apis = config_parser_get_apis
exports.config_parser_get_help_txt = config_parser_get_help_txt
exports.config_parser_check_config = config_parser_check_config
exports.config_parser_supports_pos_args = config_parser_supports_pos_args
