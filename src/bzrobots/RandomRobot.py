import bzrobot
import random

class RandomRobot(bzrobot.BZAdvancedRobot):
    def initialize(self):
        self.setCompatability(False)

    def update(self):
        self.setFire()

        self.setAhead(random.uniform(10, 20))

        while True:
            self.fireAndExecute()
            if self.getDistanceRemaining() <= 0:
                break

        self.setTurnLeft(random.uniform(1, 90))
        while True:
            self.fireAndExecute()
            if self.getTurnRemaining() <= 0:
                break

    def onHitWall(self, hitwallevent):
        self.setAhead(-50)
        self.execute()

    def fireAndExecute(self):
        if self.getGunHeat() <= 0:
            self.setFire()
        self.execute()

def create():
    return RandomRobot()
