--
-- _make.lua
-- Define the makefile action(s).
-- Copyright (c) 2002-2010 Jason Perkins and the Premake project
--

	_MAKE = { }
	premake.make = { }

--
-- Escape a string so it can be written to a makefile.
--

	function _MAKE.esc(value)
		local result
		if (type(value) == "table") then
			result = { }
			for _,v in ipairs(value) do
				table.insert(result, _MAKE.esc(v))
			end
			return result
		else
			-- handle simple replacements
			result = value:gsub("\\", "\\\\")
			result = result:gsub(" ", "\\ ")
			result = result:gsub("%(", "\\%(")
			result = result:gsub("%)", "\\%)")

			-- leave $(...) shell replacement sequences alone
			result = result:gsub("$\\%((.-)\\%)", "$%(%1%)")
			return result
		end
	end



--
-- Rules for file ops based on the shell type. Can't use defines and $@ because
-- it screws up the escaping of spaces and parethesis (anyone know a solution?)
--

	function premake.make_copyrule(source, target)
		_p('%s: %s', target, source)
		_p('\t@echo Copying $(notdir %s)', target)
		_p('ifeq (posix,$(SHELLTYPE))')
		_p('\t$(SILENT) cp -fR %s %s', source, target)
		_p('else')
		_p('\t$(SILENT) copy /Y $(subst /,\\\\,%s) $(subst /,\\\\,%s)', source, target)
		_p('endif')
	end

	function premake.make_mkdirrule(var)
		_p('\t@echo Creating %s', var)
		_p('ifeq (posix,$(SHELLTYPE))')
		_p('\t$(SILENT) mkdir -p %s', var)
		_p('else')
		_p('\t$(SILENT) mkdir $(subst /,\\\\,%s)', var)
		_p('endif')
		_p('')
	end



--
-- Get the makefile file name for a solution or a project. If this object is the
-- only one writing to a location then I can use "Makefile". If more than one object
-- writes to the same location I use name + ".make" to keep it unique.
--

	function _MAKE.getmakefilename(this, searchprjs)
		-- how many projects/solutions use this location?
		local count = 0
		for sln in premake.solution.each() do
			if (sln.location == this.location) then count = count + 1 end
			if (searchprjs) then
				for _,prj in ipairs(sln.projects) do
					if (prj.location == this.location) then count = count + 1 end
				end
			end
		end

		if (count == 1) then
			return "Makefile"
		else
			return this.name .. ".make"
		end
	end


--
-- Returns a list of object names, properly escaped to be included in the makefile.
--

	function _MAKE.getnames(tbl)
		local result = table.extract(tbl, "name")
		for k,v in pairs(result) do
			result[k] = _MAKE.esc(v)
		end
		return result
	end


--
-- Register the "gmake" action
--

	newaction {
		trigger         = "gmake",
		shortname       = "GNU Make",
		description     = "Generate GNU makefiles for POSIX, MinGW, and Cygwin",

		valid_kinds     = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib" },

		valid_languages = { "C", "C++", "C#" },

		valid_tools     = {
			cc     = { "gcc" },
			dotnet = { "mono", "msnet", "pnet" },
		},

		onsolution = function(sln)
			premake.generate(sln, _MAKE.getmakefilename(sln, false), premake.make_solution)
		end,

		onproject = function(prj)
			local makefile = _MAKE.getmakefilename(prj, true)
			if premake.isdotnetproject(prj) then
				premake.generate(prj, makefile, premake.make_csharp)
			else
				premake.generate(prj, makefile, premake.make_cpp)
			end
		end,

		oncleansolution = function(sln)
			premake.clean.file(sln, _MAKE.getmakefilename(sln, false))
		end,

		oncleanproject = function(prj)
			premake.clean.file(prj, _MAKE.getmakefilename(prj, true))
		end
	}
