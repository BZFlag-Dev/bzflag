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

#include "ViewItemScoreboard.h"
#include "LocalPlayer.h"
#include "RemotePlayer.h"
#include "DeadPlayer.h"
#include "World.h"
#include "Team.h"
#include "Flag.h"
#include "network.h"
#include "bzfgl.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

//
// sorting functions
//

static int tankScoreCompare(const void* _a, const void* _b)
{
	RemotePlayer* a = World::getWorld()->getPlayer(*(int*)_a);
	RemotePlayer* b = World::getWorld()->getPlayer(*(int*)_b);

	return b->getScore() - a->getScore();
}

static int teamScoreCompare(const void* _c, const void* _d)
{
	Team* c= World::getWorld()->getTeams() + *(int*)_c;
	Team* d= World::getWorld()->getTeams() + *(int*)_d;

	return (d->won - d->lost) - (c->won - c->lost);
}


//
// ViewItemScoreboardPlayerFormatter
//

class ViewItemScoreboardPlayerFormatter : public ViewItemScoreboard::Formatter {
	public:
		ViewItemScoreboardPlayerFormatter(const Player* _player) :
								player(_player) { }
		~ViewItemScoreboardPlayerFormatter() { }

		// Formatter overrides
		virtual const float*		getColor();
		virtual BzfString				format(const ViewItemScoreboard::Part&);

	private:
		const Player*	player;
};

const float*				ViewItemScoreboardPlayerFormatter::getColor()
{
	return Team::getRadarColor(player->getTeam());
}

BzfString				ViewItemScoreboardPlayerFormatter::format(
								const ViewItemScoreboard::Part& part)
{
	switch (part.item) {
		default:
		case ViewItemScoreboard::Part::None:
			return part.format;

		case ViewItemScoreboard::Part::Callsign:
			return BzfString::format(part.format.c_str(), player->getCallSign());

		case ViewItemScoreboard::Part::EMail:
			return BzfString::format(part.format.c_str(), player->getEmailAddress());

		case ViewItemScoreboard::Part::ID:
			return BzfString::format(part.format.c_str(),
								inet_ntoa(player->getId().serverHost),
								ntohs(player->getId().port),
								ntohs(player->getId().number));

		case ViewItemScoreboard::Part::Flag:
			if (player->getFlag() == NoFlag)
				return BzfString::format(part.format.c_str(), "");
			else
				return BzfString::format(part.format.c_str(),
								Flag::getName(player->getFlag()));

		case ViewItemScoreboard::Part::FlagAbbr:
			if (player->getFlag() == NoFlag)
				return BzfString::format(part.format.c_str(), "");
			else
				return BzfString::format(part.format.c_str(),
								Flag::getAbbreviation(player->getFlag()));

		case ViewItemScoreboard::Part::Score:
			return BzfString::format(part.format.c_str(), player->getScore());

		case ViewItemScoreboard::Part::Wins:
			return BzfString::format(part.format.c_str(), player->getWins());

		case ViewItemScoreboard::Part::Losses:
			return BzfString::format(part.format.c_str(), player->getLosses());

		case ViewItemScoreboard::Part::LocalWins:
			return BzfString::format(part.format.c_str(), player->getLocalWins());

		case ViewItemScoreboard::Part::LocalLosses:
			return BzfString::format(part.format.c_str(), player->getLocalLosses());
	}
}


//
// ViewItemScoreboardTeamFormatter
//

class ViewItemScoreboardTeamFormatter : public ViewItemScoreboard::Formatter {
	public:
		ViewItemScoreboardTeamFormatter(TeamColor _teamColor) :
								teamColor(_teamColor) { }
		~ViewItemScoreboardTeamFormatter() { }

		// Formatter overrides
		virtual const float*		getColor();
		virtual BzfString				format(const ViewItemScoreboard::Part&);

	private:
		TeamColor				teamColor;
};

const float*				ViewItemScoreboardTeamFormatter::getColor()
{
	return Team::getRadarColor(teamColor);
}

BzfString				ViewItemScoreboardTeamFormatter::format(
								const ViewItemScoreboard::Part& part)
{
	switch (part.item) {
		default:
			return part.format;

		case ViewItemScoreboard::Part::Callsign:
			return BzfString::format(part.format.c_str(),
								Team::getName(teamColor).c_str());

		case ViewItemScoreboard::Part::Score: {
			const Team& team = World::getWorld()->getTeam(teamColor);
			return BzfString::format(part.format.c_str(), team.won - team.lost);
		}

		case ViewItemScoreboard::Part::Wins: {
			const Team& team = World::getWorld()->getTeam(teamColor);
			return BzfString::format(part.format.c_str(), team.won);
		}

		case ViewItemScoreboard::Part::Losses: {
			const Team& team = World::getWorld()->getTeam(teamColor);
			return BzfString::format(part.format.c_str(), team.lost);
		}

		case ViewItemScoreboard::Part::Number: {
			const Team& team = World::getWorld()->getTeam(teamColor);
			return BzfString::format(part.format.c_str(), team.activeSize);
		}
	}
}


//
// ViewItemScoreboardTitleFormatter
//

class ViewItemScoreboardTitleFormatter : public ViewItemScoreboard::Formatter {
	public:
		ViewItemScoreboardTitleFormatter(const float* _color) :
								color(_color) { }
		~ViewItemScoreboardTitleFormatter() { }

		// Formatter overrides
		virtual const float*		getColor();
		virtual BzfString				format(const ViewItemScoreboard::Part&);

	private:
		const float*		color;
};

const float*				ViewItemScoreboardTitleFormatter::getColor()
{
	return color;
}

BzfString				ViewItemScoreboardTitleFormatter::format(
								const ViewItemScoreboard::Part& part)
{
	return part.format;
}


//
// ViewItemScoreboard
//

ViewItemScoreboard::ViewItemScoreboard() :
								shadow(false),
								showTeams(false)
{
	// do nothing
}

ViewItemScoreboard::~ViewItemScoreboard()
{
	// do nothing
}

void					ViewItemScoreboard::setTitleFormat(
								const BzfString& format)
{
	makeParts(titleParts, format);
}

void					ViewItemScoreboard::setFormat(const BzfString& format)
{
	makeParts(lineParts, format);
}

void					ViewItemScoreboard::setShadow(bool _shadow)
{
	shadow = _shadow;
}

void					ViewItemScoreboard::setShowTeams(bool _showTeams)
{
	showTeams = _showTeams;
}

bool					ViewItemScoreboard::onPreRender(
								float, float, float, float)
{
	LocalPlayer* myTank = LocalPlayer::getMyTank();
	return (myTank != NULL && World::getWorld() != NULL);
}

void					ViewItemScoreboard::onPostRender(
								float, float, float w, float h)
{
	LocalPlayer* myTank = LocalPlayer::getMyTank();

	// get line spacing
	const OpenGLTexFont& font = getState().font;
	float spacing = -font.getSpacing();

	// set initial position
	float x = 0.0f;
	float y = h + spacing - font.getAscent();

	// begin drawing
	OpenGLGState::resetState();

	// title line
	ViewItemScoreboardTitleFormatter formatter(getState().color.color);
	drawLine(titleParts, formatter, x, y, w, 1.0f);
	y += spacing;

	if (showTeams) {
		// sort teams
		int i, teams[NumTeams];
		for (i = 0; i < NumTeams; ++i)
			teams[i] = i;
		qsort(teams, NumTeams, sizeof(int), teamScoreCompare);

		// draw teams
		for (i = 0; i < NumTeams; ++i) {
			const Team* team = World::getWorld()->getTeams() + i;
			if (team->activeSize == 0)
				continue;
			ViewItemScoreboardTeamFormatter formatter(static_cast<TeamColor>(i));
			drawLine(lineParts, formatter, x, y, w, 1.0f);
			y += spacing;
		}
	}
	else {
		const int maxPlayers = World::getWorld()->getMaxPlayers();
		int i;

		// my tank
		ViewItemScoreboardPlayerFormatter formatter(myTank);
		drawLine(lineParts, formatter, x, y, w, 1.0f);
		y += spacing;

		// sort other live players
		int numPlayers = 0;
		int* players = new int[maxPlayers];
		for (i = 0; i < maxPlayers; ++i)
			if (World::getWorld()->getPlayer(i) != NULL)
				players[numPlayers++] = i;
		qsort(players, numPlayers, sizeof(int), tankScoreCompare);

		// draw live players
		for (i = 0; i < numPlayers; ++i) {
			ViewItemScoreboardPlayerFormatter formatter(
								World::getWorld()->getPlayer(players[i]));
			drawLine(lineParts, formatter, x, y, w, 1.0f);
			y += spacing;
		}
		delete[] players;

		// draw dead players
		const int maxDeadPlayers = World::getWorld()->getMaxDeadPlayers();
		DeadPlayer** deadPlayers = World::getWorld()->getDeadPlayers();
		for (i = 0; i < maxDeadPlayers; i++) {
			if (!deadPlayers[i])
				continue;
			ViewItemScoreboardPlayerFormatter formatter(deadPlayers[i]);
			drawLine(lineParts, formatter, x, y, w, 0.5f);
			y += spacing;
		}
	}
}

void					ViewItemScoreboard::drawLine(
								const Parts& parts,
								Formatter& formatter,
								float x, float y, float fullWidth,
								float colorScale)
{
	const OpenGLTexFont& font = getState().font;
	const float* srcColor = formatter.getColor();
	float color[3];
	color[0] = colorScale * srcColor[0];
	color[1] = colorScale * srcColor[1];
	color[2] = colorScale * srcColor[2];

	unsigned int n = parts.size();
	for (unsigned int i = 0; i < n; ++i) {
		const Part& part = parts[i];

		// format field string
		BzfString msg = formatter.format(part);

		// get field width
		float w = font.getWidth(msg);

		// account for shadow
		if (shadow)
			w += 1.0f;

		// compute position of field
		if (part.useOffset)
			x = part.offset.get(fullWidth);

		// draw it
		if (shadow) {
			glColor3f(0.0f, 0.0f, 0.0f);
			font.draw(msg, x, y - 1.0f);
			font.draw(msg, x + 1.0f, y);
			font.draw(msg, x + 1.0f, y - 1.0f);
		}
		glColor3fv(color);
		font.draw(msg, x, y);

		// advance position
		x += w;
	}
}

void					ViewItemScoreboard::makeParts(
								Parts& parts,
								const BzfString& fmt)
{
	parts.clear();

	const char* scan = fmt.c_str();
	while (*scan != '\0') {
		Part part;
		part.item       = Part::None;
		part.useOffset  = false;

		// read position if present
		if (*scan == '@') {
			++scan;
			if (*scan != '@') {
				float size;
				char type;
				if (sscanf(scan, "%f%c", &size, &type) == 2) {
					if (type == 'p') {
						part.offset.pixel    = size;
						part.offset.fraction = 0.0f;
						part.useOffset = true;
					}
					else if (type == '%') {
						part.offset.pixel    = 0.0f;
						part.offset.fraction = 0.01f * size;
						part.useOffset = true;
					}
				}

				// skip past position in string if it was read correctly
				if (part.useOffset) {
					while (*scan != type)
						++scan;
					++scan;
				}
			}
			else {
				// literal @
				part.format += "@";
				++scan;
			}
		}

		// read stuff up to next format or position character
		while (*scan != '\0') {
			if (*scan != '%' && *scan != '@') {
				part.format.append(scan, 1);
				++scan;
			}
			else if (scan[0] == '%' && scan[1] == '%') {
				part.format.append(scan, 1);
				scan += 2;
			}
			else if (scan[0] == '@' && scan[1] == '@') {
				part.format.append(scan, 1);
				scan += 2;
			}
			else {
				break;
			}
		}

		// decode format
		if (*scan == '%') {
			int width = 0;
			bool alignRight = false;
			++scan;

			// check for trailing %
			if (*scan == '\0') {
				part.format += "%";
				goto done;
			}

			// check for alignment
			if (*scan == ' ') {
				alignRight = true;
				++scan;
				if (*scan == '\0')
					goto done;
			}

			// check for width
			if (isdigit(*scan)) {
				char* end;
				width = static_cast<int>(strtol(scan, &end, 10));
				if (end == scan)
					goto done;
				scan = end;
			}

			// check type
			switch (*scan) {
				case 'c':
					part.item = Part::Callsign;
					break;

				case 'e':
					part.item = Part::EMail;
					break;

				case 'i':
					part.item = Part::ID;
					break;

				case 'f':
					part.item = Part::Flag;
					break;

				case 'F':
					part.item = Part::FlagAbbr;
					break;

				case 's':
					part.item = Part::Score;
					break;

				case 'w':
					part.item = Part::Wins;
					break;

				case 'l':
					part.item = Part::Losses;
					break;

				case 'W':
					part.item = Part::LocalWins;
					break;

				case 'L':
					part.item = Part::LocalLosses;
					break;

				case 'n':
					part.item = Part::Number;
					break;

				default:
					goto done;
			}
			++scan;

			// finish off format
			switch (part.item) {
				default:
				case Part::None:
					break;

				// simple strings
				case Part::Callsign:
				case Part::EMail:
				case Part::Flag:
				case Part::FlagAbbr:
					if (width > 0)
						part.format += BzfString::format("%%%s%ds",
								alignRight ? " " : "", width);
					else
						part.format += "%s";
					break;

				// simple integers
				case Part::Score:
				case Part::Wins:
				case Part::Losses:
				case Part::LocalWins:
				case Part::LocalLosses:
				case Part::Number:
					if (width > 0)
						part.format += BzfString::format("%%%s%dd",
								alignRight ? " " : "", width);
					else
						part.format += "%d";
					break;

				case Part::ID:
					if (width > 8)
						part.format += BzfString::format("%%%s%ds:%%04x-%%02x",
								alignRight ? " " : "", width - 8);
					else
						part.format += "%s:%04x-%02x";
					break;
			}

done:
      // must have statement for label
      ;
    }

    parts.push_back(part);
	}
}


//
// ViewItemScoreboardReader
//

ViewItemScoreboardReader::ViewItemScoreboardReader() :
								item(NULL)
{
	// do nothing
}

ViewItemScoreboardReader::~ViewItemScoreboardReader()
{
	if (item != NULL)
		item->unref();
}

ViewTagReader* 			ViewItemScoreboardReader::clone() const
{
	return new ViewItemScoreboardReader;
}

View*					ViewItemScoreboardReader::open(
								const ConfigReader::Values& values)
{
	assert(item == NULL);

	// get parameters
	bool team = false, shadow = false;
	BzfString title, line;
	ConfigReader::Values::const_iterator index;
	index = values.find("team");
	if (index != values.end())
		team = (index->second == "yes");
	index = values.find("shadow");
	if (index != values.end())
		shadow = (index->second == "yes");
	index = values.find("title");
	if (index != values.end())
		title = index->second;
	index = values.find("format");
	if (index != values.end())
		line = index->second;

	// create item
	item = new ViewItemScoreboard;
	item->setTitleFormat(title);
	item->setFormat(line);
	item->setShadow(shadow);
	item->setShowTeams(team);

	return item;
}

void					ViewItemScoreboardReader::close()
{
	assert(item != NULL);
	item = NULL;
}
