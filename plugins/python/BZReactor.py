#!/usr/bin/env python
#
# Copyright (C) 2005 David Trowbridge
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

from twisted.internet import selectreactor
import BZFlag, BZFlag.Event

class BZReactor (selectreactor.SelectReactor):
    """Twisted reactor that uses the bzfs main loop
    """

    def run (self, installSignalHandlers=1):
        self.startRunning (installSignalHandlers = installSignalHandlers)
        BZFlag.Events[BZFlag.Event.Tick].append (self.simulate)
        BZFlag.SetMaxWaitTime (0.1)

    def setMaxDelay (self, time):
        BZFlag.SetMaxWaitTime (time)

    def simulate (self, time):
        """Run simulation loops
        """
        self.iterate ()
