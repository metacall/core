#!/usr/bin/ruby

require 'find'
require 'rbconfig'
require 'fiddle'

module MetaCall
	extend self

	private

	def find_files_recursively(root_dir, pattern)
		regex = Regexp.new(pattern)
		matches = []

		if Dir.exist?(root_dir)
			Find.find(root_dir) do |path|
				matches << path if File.file?(path) && regex.match?(File.basename(path))
			end
		end

		matches
	end

	def platform_install_paths
		host_os = RbConfig::CONFIG['host_os']

		case host_os
		when /mswin|mingw|cygwin/
			{
				paths: [ File.join(ENV['LOCALAPPDATA'].to_s, 'MetaCall', 'metacall') ],
				name: 'metacall\.dll'
			}
		when /darwin/
			{
				paths: [ '/opt/homebrew/lib/', '/usr/local/lib/' ],
				name: 'libmetacall\.dylib'
			}
		when /linux/
			{
				paths: [ '/usr/local/lib/', '/gnu/lib/' ],
				name: 'libmetacall\.so'
			}
		else
			raise "Platform #{host_os} not supported"
		end
	end

	def search_paths
		custom_path = ENV['METACALL_INSTALL_PATH']

		if custom_path
			{
				paths: [ custom_path ],
				name: '^(lib)?metacall(d)?\.(so|dylib|dll)$'
			}
		else
			platform_install_paths
		end
	end

	def find_library
		search_data = search_paths

		search_data[:paths].each do |path|
			files = find_files_recursively(path, search_data[:name])
			return files.first unless files.empty?
		end

		raise LoadError, <<~ERROR
			MetaCall library not found. If you have it in a special folder, define it using the METACALL_INSTALL_PATH environment variable.
			Searched in: #{search_data[:paths].join(', ')}

			If you do not have it installed, you have three options:
				1) Go to https://github.com/metacall/install and install it.
				2) Contribute to https://github.com/metacall/distributable by providing support for your platform and architecture.
				3) Be a x10 programmer and compile it yourself, then define the install folder using METACALL_INSTALL_PATH.
		ERROR
	end

	def metacall_module_load
		# Check if already loaded
		if defined?(MetaCallRbLoaderPort)
			return MetaCallRbLoaderPort
		end

		# Set environment variable for the host
		ENV['METACALL_HOST'] = 'rb'

		# Find and load the MetaCall shared library
		library_path = find_library

		begin
			# Load the shared library globally
			Fiddle::Handle.new(library_path, Fiddle::RTLD_GLOBAL | Fiddle::RTLD_NOW)
		rescue Fiddle::DLError => e
			raise LoadError, "Failed to load MetaCall library at #{library_path}: #{e.message}"
		end

		# Check again if the port was loaded
		if defined?(MetaCallRbLoaderPort)
			return MetaCallRbLoaderPort
		else
			raise LoadError, 'MetaCall was found but failed to load MetaCallRbLoaderPort'
		end
	end

	# Initialize the MetaCall Ruby Port
	metacall_module_load

	public

	def metacall_load_from_file(tag, paths)
		MetaCallRbLoaderPort.metacall_load_from_file(tag, paths)
	end

	def metacall_load_from_memory(tag, script)
		MetaCallRbLoaderPort.metacall_load_from_memory(tag, script)
	end

	def metacall(function_name, *args)
		MetaCallRbLoaderPort.metacall(function_name, *args)
	end

	def metacall_inspect()
		MetaCallRbLoaderPort.metacall_inspect()
	end

end
