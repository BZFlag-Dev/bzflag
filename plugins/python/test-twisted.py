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

from BZReactor import BZReactor
from twisted.web import server
from Nouvelle import tag, Twisted

class Hello(Twisted.Page):
    isLeaf = 1
    document = tag('html')[
                   tag('head') [
                       tag('title')[ "Hi" ],
                   ],
                   tag('body') [
                       tag('h3')[ "Hello World!" ],
                   ],
               ]

reactor = BZReactor ()
root = Hello ()
site = server.Site(root)
reactor.listenTCP(8080, site)
reactor.run ()
