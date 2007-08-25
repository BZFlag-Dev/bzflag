import bzrobot

class TestRobot(bzrobot.BZAdvancedRobot):
    def initialize(self):
        self.setCompatability(False)
        self.getPlayers()

        i = 0
        for player in self.players:
            print "player %i: %s" % (i, player.callsign)
            i += 1

    def update(self):
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

    def onHitWall(self, hitwallevent):
        self.setAhead(-20)
        self.execute()

def create():
    return TestRobot()
