/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_VIEWITEMSCOREBOARD_H
#define BZF_VIEWITEMSCOREBOARD_H

#include "View.h"

class Player;
class Team;

class ViewItemScoreboard : public View {
public:
	ViewItemScoreboard();

	// format string is an arbitrary string of printable characters
	// (including space) with any number of field and position
	// specifiers.
	//
	// a field specifier is
	//    % [space] [<width>] <type>
	// the optional space indicates right alignment.  the optional
	// width is the minimum width for the field.  the type is what
	// to draw (see the Item enum below).  a %% becomes a literal %.
	//
	// position specifiers are:
	//    @ <position> <unit>
	// the unit is p for pixels or % for percent of the view width.
	// the output position is moved by a position specifier.  text
	// then flows to the left.  an @@ becomes a literal @.
	//
	// the title format should not have any field specifiers.
	void				setTitleFormat(const BzfString& format);
	void				setFormat(const BzfString& format);
	void				setShadow(bool);
	void				setShowTeams(bool);

protected:
	virtual ~ViewItemScoreboard();

	// View overrides
	virtual bool		onPreRender(float x, float y, float w, float h);
	virtual void		onPostRender(float x, float y, float w, float h);

public:
	struct Part {
	public:
		enum Item {			// player	team
			None,
			Callsign,		// %c		%c (team name)
			EMail,			// %e
			ID,				// %i
			Flag,			// %f
			FlagAbbr,		// %F
			Score,			// %s		%s
			Wins,			// %w		%w
			Losses,			// %l		%l
			LocalWins,		// %W
			LocalLosses,	// %L
			Number			//			%n (number of players)
		};

		bool			useOffset;
		Item			item;
		BzfString		format;
		ViewSize		offset;
	};

	class Formatter {
	public:
		Formatter() { }
		virtual ~Formatter() { }

		virtual const float*	getColor() = 0;
		virtual BzfString		format(const Part&) = 0;
	};

private:
	typedef std::vector<Part> Parts;

	void				makeParts(Parts&, const BzfString&);
	void				drawLine(const Parts&, Formatter&,
								float x, float y, float fullWidth,
								float colorScale);

private:
	Parts				titleParts;
	Parts				lineParts;
	bool				shadow;
	bool				showTeams;
};

class ViewItemScoreboardReader : public ViewTagReader {
public:
	ViewItemScoreboardReader();
	virtual ~ViewItemScoreboardReader();

	// ViewItemReader overrides
	virtual ViewTagReader* clone() const;
	virtual View*		open(const ConfigReader::Values&);
	virtual void		close();

private:
	ViewItemScoreboard*	item;
};

#endif
