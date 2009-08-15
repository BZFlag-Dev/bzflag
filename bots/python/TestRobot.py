# -*- coding: utf-8 -*-
import bzrobot

def create():
    return TestRobot()

class TestRobot(bzrobot.AdvancedRobot):

    def run(self):
        while True:
            # This kind of try/except block around your update can be
            # VERY useful when debugging, since SWIG (not svn HEAD, afaik)
            # has some issues with handling Python-generated exceptions. :)
            try:
                self.setFire()
                self.setAhead(20)

                while True:
                    if self.getGunHeat() <= 0:
                        self.setFire()
                    self.execute()
                    if self.getDistanceRemaining() <= 0:
                        break

                self.setTurnLeft(90)
                while True:
                    self.execute()
                    if self.getTurnRemaining() <= 0:
                        break

            except Exception, e:
                print e

    def onSpawn(self, spawnevent):
	print "TestRobot: Spawn"

    def onDeath(self, deathevent):
	print "TestRobot: Death"


