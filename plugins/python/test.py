#!/usr/bin/env python
#
# Copyright (C) 2006 David Trowbridge
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#

import BZFlag
import BZFlag.Event
import BZFlag.Team

BZFlag.SendTextMessage (0, 0, 'hello')

print BZFlag.__dict__
print BZFlag.Event.__dict__
print BZFlag.Team.__dict__

def tick(time):
    pass

def join():
    print BZFlag.Players

def part():
    print BZFlag.Players

BZFlag.Events[BZFlag.Event.Tick].append (tick)
BZFlag.Events[BZFlag.Event.PlayerJoin].append (join)
BZFlag.Events[BZFlag.Event.PlayerPart].append (part)

print BZFlag.BZDB
print BZFlag.BZDB['_worldSize']

BZFlag.SetMaxWaitTime (0.5);
