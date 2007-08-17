import bzrobot

class TestRobot(bzrobot.BZAdvancedRobot):
    def initialize(self):
        self.setCompatability(False)

    def update(self):
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

    def onHitWall(self, hitwallevent):
        self.setAhead(0)

def create():
    return TestRobot()
