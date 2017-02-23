#!/usr/bin/ruby

IO.popen('./bin/rails server') do |io|
	puts 'Launching rails server on process..', io.pid

	Signal.trap('INT') {
		Process.kill('INT', io.pid)

		io.close

		Signal.trap('INT', 'DEFAULT')

		Process.kill('INT', 0)
	}

	begin
		while line = io.gets do
			puts line
		end
	rescue SystemExit, Interrupt
		exit 0
	end
end
