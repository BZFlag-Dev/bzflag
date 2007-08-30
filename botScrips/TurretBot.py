from bzrobot import BZAdvancedRobot
from time import sleep
from random import randint

# This bot just stays stationary, and fires at a random player.
class TurretBot(BZAdvancedRobot):
    def printplayers(self):
        try:
            i = 0
            print "self: (%f, %f, %f) angle: %f" % (self.getX(), self.getY(), self.getZ(), self.getHeading())
            for player in self.players:
                print "player %i: %s - (%f, %f, %f) angle: %f, bearing: %f" % (i, player.callsign, player.position[0], player.position[1], player.position[2], player.angle, self.getBearing(player))
                i += 1
        except Exception, e:
            print e

    def initialize(self):
        try:
            self.setCompatability(False)
        except Exception, e:
            print e

    def update(self):
        # This kind of try/except block around your update can be
        # VERY useful when debugging, since SWIG (not svn HEAD, afaik)
        # has some issues with handling Python-generated exceptions. :)
        try:
            self.getPlayers()
            self.printplayers()

            if len(self.players) == 0:
                sleep(2)
                return
    
            target = self.players[randint(0, len(self.players) - 1)]
            print "Targeting player '%s'" % (target.callsign)
            
            self.setTurnLeft(self.getBearing(target))
            while True:
                self.execute()
                if self.getTurnRemaining() <= 1:
                    break

            self.setFire()
            self.execute()
        except Exception, e:
            print e

def create():
    return TurretBot()
