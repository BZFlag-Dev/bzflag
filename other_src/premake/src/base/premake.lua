--
-- premake.lua
-- High-level processing functions.
-- Copyright (c) 2002-2009 Jason Perkins and the Premake project
--


--
-- Open a file for output, and call a function to actually do the writing.
-- Used by the actions to generate solution and project files.
--
-- @param obj
--    A solution or project object; will be based to the callback function.
-- @param filename
--    The output filename; see the docs for premake.project.getfilename()
--    for the expected format.
-- @param callback
--    The function responsible for writing the file, should take a solution
--    or project as a parameters.
--

  -- BZFlag customization
	function premake.generate(obj, filename, callback)
		filename = premake.project.getfilename(obj, filename)
		printf("Generating %s...", filename)

		local olddata = nil
		do
			oldfile = io.open(filename, "rb")
			if (oldfile) then
				olddata = oldfile:read("*a")
				oldfile:close()
			end
		end

		local f, err
		if (olddata) then
			f, err = io.tmpfile()
		else
			f, err = io.open(filename, "wb")
		end
		if (not f) then
			error(err, 0)
		end

		-- set the default output stream
		io.output(f)

		-- run the callback
		callback(obj)

		if (not olddata) then
			f:close()
		else
			-- read the tmpfile
			f:seek("set")
			local newdata = f:read("*a")
			f:close()

			if (newdata == olddata) then
				print("           " .. filename .. " is unchanged")
			else
				local wf, err = io.open(filename, "wb")
				if (not wf) then
					error(err, 0)
				end
				wf:write(newdata)
				wf:close()
			end
		end
	end
