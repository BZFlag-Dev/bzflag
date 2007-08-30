from bzrobot import BZAdvancedRobot
from time import time, sleep
from copy import deepcopy

# This bot is currently not doing anything in particular, but is meant to
# eventually use a simple algorithm to predict linear movement, and fire towards
# the predicted point.
# (By solving the equation:
#  t**2 * |v_b|**2 = t(v_x*dx*2+2*v_y*dy) + t**2(v_x**2+v_y**2)+dx**2+dy**2,
# then substituting t into the equation for the bots position)
class SimpleRobot(BZAdvancedRobot):
    def printplayers(self):
        self.getPlayers()
        i = 0
        print "self: (%f, %f, %f) angle: %f" % (self.getX(), self.getY(), self.getZ(), self.getHeading())
        for player in self.players:
            print "player %i: %s - (%f, %f, %f) angle: %f, bearing: %f" % (i, player.callsign, player.position[0], player.position[1], player.position[2], player.angle, self.getBearing(player))
            i += 1

    def initialize(self):
        print self.methods()
        try:
            self.setCompatability(False)
            self.lastUpdate = time()
            self.printplayers()
        except Exception, e:
            print e

    def update(self):
        # This kind of try/except block around your update can be
        # VERY useful when debugging, since SWIG (not svn HEAD, afaik)
        # has some issues with handling Python-generated exceptions. :)
        try:
            if time() > self.lastUpdate + 3:
                self.lastUpdate = time()
                self.printplayers()

            if len(self.players) > 0:
                self.setTurnLeft(self.getBearing(self.players[0]))
            while True:
                self.execute()
                if self.getTurnRemaining() <= 0:
                    break
            sleep(1)

        except Exception, e:
            print e

    def onHitWall(self, hitwallevent):
        self.setAhead(-20)
        self.execute()

    def methods(self):
        return [attr for attr in dir(self) if callable(getattr(self, attr, False))]

def create():
    return SimpleRobot()
