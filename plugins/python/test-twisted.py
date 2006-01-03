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

import BZFlag, Nouvelle
from BZReactor import BZReactor
from twisted.web import server
from Nouvelle import tag, place, Twisted

class MyTable(Nouvelle.ResortableTable):
    tableTag = tag ('table', border='1')

class Hello(Twisted.Page):
    isLeaf = 1
    document = tag('html')[
                   tag('head') [
                       tag('title')[ "Hi" ],
                   ],
                   tag('body') [
                       tag('h3')[ "Hello World!" ],
                       tag('p') [ place("dataTable") ]
                   ],
               ]

    def render_dataTable (self, context):
        data = []

        for id in BZFlag.Players.keys ():
            player = BZFlag.Players[id]
            data.append ([id, player.callsign, player.wins - player.losses, player.team])

        return MyTable (data, [
        Nouvelle.IndexedColumn ('id', 0),
        Nouvelle.IndexedColumn ('callsign', 1),
        Nouvelle.IndexedColumn ('score', 2),
        Nouvelle.IndexedColumn ('team', 3),
        ], id='players')

reactor = BZReactor ()

root = Hello ()
site = server.Site(root)
reactor.listenTCP(8080, site)

reactor.setMaxDelay (0.05)
reactor.run ()
