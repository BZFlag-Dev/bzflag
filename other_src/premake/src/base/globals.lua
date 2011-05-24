--
-- globals.lua
-- Global tables and variables, replacements and extensions to Lua's global functions.
-- Copyright (c) 2002-2009 Jason Perkins and the Premake project
--


-- A top-level namespace for support functions

	premake = { }


-- The list of supported platforms; also update list in cmdline.lua

	premake.platforms =
	{
		Native =
		{
			cfgsuffix       = "",
		},
		x32 =
		{
			cfgsuffix       = "32",
		},
		x64 =
		{
			cfgsuffix       = "64",
		},
		Universal =
		{
			cfgsuffix       = "univ",
		},
		Universal32 =
		{
			cfgsuffix       = "univ32",
		},
		Universal64 =
		{
			cfgsuffix       = "univ64",
		},
		PS3 =
		{
			cfgsuffix       = "ps3",
			iscrosscompiler = true,
			nosharedlibs    = true,
			namestyle       = "PS3",
		},
		Xbox360 =
		{
			cfgsuffix       = "xbox360",
			iscrosscompiler = true,
			namestyle       = "windows",
		},
	}


--
-- A replacement for Lua's built-in dofile() function, this one sets the
-- current working directory to the script's location, enabling script-relative
-- referencing of other files and resources.
--

	local builtin_dofile = dofile
	function dofile(fname)
		-- remember the current working directory and file; I'll restore it shortly
		local oldcwd = os.getcwd()
		local oldfile = _SCRIPT

		-- if the file doesn't exist, check the search path
		if (not os.isfile(fname)) then
			local path = os.pathsearch(fname, _OPTIONS["scripts"], os.getenv("PREMAKE_PATH"))
			if (path) then
				fname = path.."/"..fname
			end
		end

		-- use the absolute path to the script file, to avoid any file name
		-- ambiguity if an error should arise
		_SCRIPT = path.getabsolute(fname)

		-- switch the working directory to the new script location
		local newcwd = path.getdirectory(_SCRIPT)
		os.chdir(newcwd)

		-- run the chunk. How can I catch variable return values?
		local a, b, c, d, e, f = builtin_dofile(_SCRIPT)

		-- restore the previous working directory when done
		_SCRIPT = oldfile
		os.chdir(oldcwd)
		return a, b, c, d, e, f
	end



--
-- "Immediate If" - returns one of the two values depending on the value of expr.
--

	function iif(expr, trueval, falseval)
		if (expr) then
			return trueval
		else
			return falseval
		end
	end



--
-- A shortcut for including another "premake4.lua" file, often used for projects.
--

	function include(fname)
		return dofile(fname .. "/premake4.lua")
	end



--
-- A shortcut for printing formatted output.
--

	function printf(msg, ...)
		print(string.format(msg, unpack(arg)))
	end



--
-- An extension to type() to identify project object types by reading the
-- "__type" field from the metatable.
--

	local builtin_type = type
	function type(t)
		local mt = getmetatable(t)
		if (mt) then
			if (mt.__type) then
				return mt.__type
			end
		end
		return builtin_type(t)
	end
