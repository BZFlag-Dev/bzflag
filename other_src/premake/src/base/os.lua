--
-- os.lua
-- Additions to the OS namespace.
-- Copyright (c) 2002-2010 Jason Perkins and the Premake project
--


--
-- Same as os.execute(), but accepts string formatting arguments.
--

	function os.executef(cmd, ...)
		cmd = string.format(cmd, unpack(arg))
		return os.execute(cmd)
	end


--
-- Scan the well-known system locations for a particular library.
--

	function os.findlib(libname)
		local path, formats

		-- assemble a search path, depending on the platform
		if os.is("windows") then
			formats = { "%s.dll", "%s" }
			path = os.getenv("PATH")
		else
			if os.is("macosx") then
				formats = { "lib%s.dylib", "%s.dylib" }
				path = os.getenv("DYLD_LIBRARY_PATH")
			else
				formats = { "lib%s.so", "%s.so" }
				path = os.getenv("LD_LIBRARY_PATH") or ""

				io.input("/etc/ld.so.conf")
				if io.input() then
					for line in io.lines() do
						path = path .. ":" .. line
					end
					io.input():close()
				end
			end

			table.insert(formats, "%s")
			path = (path or "") .. ":/lib:/usr/lib:/usr/local/lib"
		end

		for _, fmt in ipairs(formats) do
			local name = string.format(fmt, libname)
			local result = os.pathsearch(name, path)
			if result then return result end
		end
	end



--
-- Retrieve the current operating system ID string.
--

	function os.get()
		return _OPTIONS.os or _OS
	end



--
-- Check the current operating system; may be set with the /os command line flag.
--

	function os.is(id)
		return (os.get():lower() == id:lower())
	end



--
-- The os.matchdirs() and os.matchfiles() functions
--

	local function domatch(result, mask, wantfiles)
		-- need to remove extraneous path info from the mask to ensure a match
		-- against the paths returned by the OS. Haven't come up with a good
		-- way to do it yet, so will handle cases as they come up
		if mask:startswith("./") then
			mask = mask:sub(3)
		end

		-- strip off any leading directory information to find out
		-- where the search should take place
		local basedir = mask
		local starpos = mask:find("%*")
		if starpos then
			basedir = basedir:sub(1, starpos - 1)
		end
		basedir = path.getdirectory(basedir)
		if (basedir == ".") then basedir = "" end

		-- recurse into subdirectories?
		local recurse = mask:find("**", nil, true)

		-- convert mask to a Lua pattern
		mask = path.wildcards(mask)

		local function matchwalker(basedir)
			local wildcard = path.join(basedir, "*")

			-- retrieve files from OS and test against mask
			local m = os.matchstart(wildcard)
			while (os.matchnext(m)) do
				local isfile = os.matchisfile(m)
				if ((wantfiles and isfile) or (not wantfiles and not isfile)) then
					local fname = path.join(basedir, os.matchname(m))
					if fname:match(mask) == fname then
						table.insert(result, fname)
					end
				end
			end
			os.matchdone(m)

			-- check subdirectories
			if recurse then
				m = os.matchstart(wildcard)
				while (os.matchnext(m)) do
					if not os.matchisfile(m) then
						local dirname = os.matchname(m)
						matchwalker(path.join(basedir, dirname))
					end
				end
				os.matchdone(m)
			end
		end

		matchwalker(basedir)
	end

	function os.matchdirs(...)
		local result = { }
		for _, mask in ipairs(arg) do
			domatch(result, mask, false)
		end
		return result
	end

	function os.matchfiles(...)
		local result = { }
		for _, mask in ipairs(arg) do
			domatch(result, mask, true)
		end
		return result
	end



--
-- An overload of the os.mkdir() function, which will create any missing
-- subdirectories along the path.
--

	local builtin_mkdir = os.mkdir
	function os.mkdir(p)
		local dir = iif(p:startswith("/"), "/", "")
		for part in p:gmatch("[^/]+") do
			dir = dir .. part

			if (part ~= "" and not path.isabsolute(part) and not os.isdir(dir)) then
				local ok, err = builtin_mkdir(dir)
				if (not ok) then
					return nil, err
				end
			end

			dir = dir .. "/"
		end

		return true
	end


--
-- Remove a directory, along with any contained files or subdirectories.
--

	local builtin_rmdir = os.rmdir
	function os.rmdir(p)
		-- recursively remove subdirectories
		local dirs = os.matchdirs(p .. "/*")
		for _, dname in ipairs(dirs) do
			os.rmdir(dname)
		end

		-- remove any files
		local files = os.matchfiles(p .. "/*")
		for _, fname in ipairs(files) do
			os.remove(fname)
		end

		-- remove this directory
		builtin_rmdir(p)
	end

