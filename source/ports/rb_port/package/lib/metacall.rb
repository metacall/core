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
				filename = File.basename(path)
				matches << path if File.file?(path) && regex.match?(filename)
			end
		end

		matches
	end

	def platform_install_paths
		host_os = RbConfig::CONFIG['host_os']

		case host_os
		when /mswin|mingw|cygwin/
			{
				paths: [ 
					File.join(ENV['LOCALAPPDATA'].to_s, 'MetaCall', 'metacall'),
				],
				name: '^metacall(d)?\.dll$'
			}
		when /darwin/
			{
				paths: [ 
					'/opt/homebrew/lib/', 
					'/usr/local/lib/', 
				],
				name: '^libmetacall(d)?\.dylib$'
			}
		when /linux/
			{
				paths: [ 
					'/usr/local/lib/', 
					'/gnu/lib/', 
				],
				name: '^libmetacall(d)?\.so(\.\d+)*$'
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
				name: '^(lib)?metacall(d)?\.(so|dylib|dll)(\.\d+)*$'
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
		
		# Find the MetaCall shared library
		library_path = find_library

		# TODO: Check if it is required
		install_dir = File.dirname(library_path)
		root_dir = File.dirname(install_dir)

		# # AUTOMATIC ENGINE BOOTSTRAPPING
		# ENV['LOADER_LIBRARY_PATH'] ||= install_dir
		# ENV['SERIAL_LIBRARY_PATH'] ||= install_dir
		# ENV['DETECTOR_LIBRARY_PATH'] ||= install_dir
		
		# config_path = File.join(root_dir, 'configurations')
		# ENV['CONFIGURATION_PATH'] ||= config_path if Dir.exist?(config_path)

		# Platform-specific environment fixes
		if RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/
			# Ruby 3+ ignores ENV['PATH'] for DLL loading. We must use SetDllDirectory 
			# to allow metacall.dll to find its plugins and dependencies.
			begin
				kernel32 = Fiddle.dlopen('kernel32.dll')
				set_dll_dir = Fiddle::Function.new(kernel32['SetDllDirectoryW'], [Fiddle::TYPE_VOIDP], Fiddle::TYPE_INT)
				
				# Add the library directory
				set_dll_dir.call(install_dir.encode('UTF-16LE'))
				
				# Add the Ruby runtime bin folder if it exists
				rb_runtime_bin = File.join(root_dir, 'runtimes', 'ruby', 'bin')
				set_dll_dir.call(rb_runtime_bin.encode('UTF-16LE')) if Dir.exist?(rb_runtime_bin)
			rescue => e
				# Fallback to PATH for older Ruby versions
				ENV['PATH'] = "#{install_dir};#{ENV['PATH']}"
			end

			# TODO: We must move this outside here
			# # Detect Python runtime bundled with MetaCall
			# unless ENV.key?('PYTHONHOME')
			# 	py_home = File.join(root_dir, 'runtimes', 'python')
			# 	if Dir.exist?(py_home)
			# 		ENV['PYTHONHOME'] = py_home
			# 		ENV['PYTHONPATH'] ||= install_dir
			# 	end
			# end
		end

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

	# When we are running MetaCall with Ruby, we should hook the at exit method
	if ENV.key?('METACALL_HOST')
		module Kernel
			alias_method :original_exit, :exit
			alias_method :original_exit_bang, :exit!

			def exit(status = true)
				if defined?(MetaCallRbLoaderPort) && MetaCall.respond_to?(:rb_loader_port_atexit)
					MetaCallRbLoaderPort.rb_loader_port_atexit
				end
				original_exit(status)
			end

			def exit!(status = true)
				if defined?(MetaCallRbLoaderPort) && MetaCall.respond_to?(:rb_loader_port_atexit)
					MetaCallRbLoaderPort.rb_loader_port_atexit
				end
				original_exit_bang(status)
			end
		end
	end

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
