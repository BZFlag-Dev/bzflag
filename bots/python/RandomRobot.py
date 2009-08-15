# -*- coding: utf-8 -*-
import bzrobot
import random

def create():
    return RandomRobot()

class RandomRobot(bzrobot.AdvancedRobot):

    def run(self):
	while True:
	    self.setFire()

	    self.setAhead(random.uniform(10, 20))

	    while True:
		if self.getGunHeat() <= 0:
		    self.fire()
		if self.getDistanceRemaining() <= 0:
		    break

	    self.setTurnLeft(random.uniform(1, 90))
	    while True:
		if self.getGunHeat() <= 0:
		    self.fire()
		if self.getTurnRemaining() <= 0:
		    break

    def onHitWall(self, hitwallevent):
        self.back(20)
        

