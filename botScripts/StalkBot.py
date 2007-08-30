from bzrobot import BZAdvancedRobot
from math import sqrt
from time import sleep

# This bot just turn towards, then drive towards, the first player to join.
class StalkBot(BZAdvancedRobot):
    def printplayers(self):
        i = 0
        print "self: (%f, %f, %f) angle: %f" % (self.getX(), self.getY(), self.getZ(), self.getHeading())
        for player in self.players:
            print "player %i: %s - (%f, %f, %f) angle: %f, bearing: %f" % (i, player.callsign, player.position[0], player.position[1], player.position[2], player.angle, self.getBearing(player))
            i += 1

    def initialize(self):
        print self.methods()
        try:
            self.setCompatability(False)
            self.target = None
        except Exception, e:
            print e

    def update(self):
        # This kind of try/except block around your update can be
        # VERY useful when debugging, since SWIG (not svn HEAD, afaik)
        # has some issues with handling Python-generated exceptions. :)
        try:
            self.getPlayers()
            self.printplayers()
    
            if self.target == None:
                if len(self.players) == 0:
                    sleep(2)
                    return
                self.target = self.players[0].callsign
            
            tank = None
            for player in self.players:
                if player.callsign == self.target:
                    tank = player
                    break

            if tank == None:
                self.target = None
                return

            bearing = self.getBearing(tank)
            if bearing > 5:
                self.setTurnLeft(bearing)
                while True:
                    self.execute()
                    if self.getTurnRemaining() <= 1:
                        break

            vector = [tank.position[0] - self.getX(), tank.position[1] - self.getY()]
            length = sqrt(vector[0]**2 + vector[1]**2)
            if length > 4:
                self.setAhead(length)
                while True:
                    self.execute()
                    if self.getDistanceRemaining() <= 1:
                        break

        except Exception, e:
            print e

    def onHitWall(self, hitwallevent):
        self.setAhead(-20)
        while True:
            self.execute()
            if self.getDistanceRemaining() <= 0:
                break

    def methods(self):
        return [attr for attr in dir(self) if callable(getattr(self, attr, False))]

def create():
    return StalkBot()
