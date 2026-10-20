#!/usr/bin/env python3
# Update the BZFlag version in some project files to match what is contained
# within version_defs.h
#
# This file is marked CC0 1.0 Universal. To view a copy of this mark, visit
#   https://creativecommons.org/publicdomain/zero/1.0/

import os
import re
import sys

# Figure out the root path of the source tree
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Shortcut to get the relative path of a file
def rel(path):
	return os.path.relpath(path, ROOT)

# Build and return the version triplet as defined in version_defs.h.
def read_defined_version():
	# Build the path and ensure the file exists
	header_path = os.path.join(ROOT, "include", "version_defs.h")
	if not os.path.isfile(header_path):
		sys.exit("ERROR: %s not found" % rel(header_path))

	# Read the contents of the file
	with open(header_path, encoding="utf-8") as f:
		text = f.read()

	# Function to parse out a single definition that tries to be tolerant of whitespace changes
	def get(name):
		m = re.search(r"^\s*#\s*define\s+%s\s+(\d+)" % name, text, re.M)
		if not m:
			sys.exit("ERROR: %s not found (or not a plain integer) in %s" % (name, rel(header_path)))
		return int(m.group(1))

	# Return the version triplet
	return "%d.%d.%d" % (get("BZ_MAJOR_VERSION"), get("BZ_MINOR_VERSION"), get("BZ_REV"))

# Apply a regex replacement pattern to a file
def update_file(path, pattern, replacement):
	# Read the contents of the file
	with open(path, encoding="utf-8") as f:
		text = f.read()

	# Run the replacement, storing a copy of the new text and a count of the matches found
	new_text, count = re.subn(pattern, replacement, text)

	# If we found 0 matches, something likely went wrong
	if count == 0:
		sys.exit("ERROR: No matches found in %s" % rel(path))

	# If the new text matches the old text, then we might have already had the files updated, or something went wrong
	if new_text == text:
		print("  NOT updated: %s had no changes" % rel(path))
	else:
		with open(path, "w", encoding="utf-8") as f:
			f.write(new_text)
		print("  updated: %s (%d replacement%s)" % (rel(path), count, "s" if count != 1 else ""))

def main():
	# Get the version triplet
	new_version = read_defined_version()
	print("Version detected: %s" % new_version)
	print()

	# Define our replacements as an array of tuples, with the absolute file path, regex pattern, and regex replacement
	replacements = [
		# BZFlag-x.y.z and BZFlag-x.y.z.app (multiple occurrences)
		(
			os.path.join(ROOT, "README"),
			r"BZFlag [0-9]+\.[0-9]+\.[0-9]+",
			r"BZFlag " + new_version
		),

		# AC_INIT([BZFlag],[x.y.z],[...],[bzflag])
		(
			os.path.join(ROOT, "configure.ac"),
			r"(\bAC_INIT\(\s*\[\s*BZFlag\s*\]\s*,\s*\[)([0-9]+\.[0-9]+\.[0-9]+)(\])",
			r"\g<1>" + new_version + r"\g<3>"
		),

		# BZFlag-x.y.z and BZFlag-x.y.z.app (multiple occurrences)
		(
			os.path.join(ROOT, "Xcode", "BZFlag.xcodeproj", "project.pbxproj"),
			r"\bBZFlag-[0-9]+\.[0-9]+\.[0-9]+",
			r"BZFlag-" + new_version
		),

		# <key>CFBundleShortVersionString</key> immediately followed by <string>x.y.z</string> on the next line
		(
			os.path.join(ROOT, "Xcode", "BZFlag-Info.plist"),
			r"(<key>\s*CFBundleShortVersionString\s*</key>\s*<string>)([0-9]+\.[0-9]+\.[0-9]+)(</string>)",
			r"\g<1>" + new_version + r"\g<3>"
		),
	]

	# First check that we can find all of these files, and bail out now if we can't
	for path, _, _ in replacements:
		if not os.path.isfile(path):
			sys.exit("ERROR: %s not found" % rel(path))

	# Now that we verified they exist, update the version number in each
	print("Updating files:")
	for path, pattern, replacement in replacements:
		update_file(path, pattern, replacement)

	# If we got this far, report success.
	print()
	print("Done! Please manually verify that the files were modified correctly.")

if __name__ == "__main__":
	main()
