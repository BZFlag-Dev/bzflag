# -*- coding: utf-8 -*-
import bzrobot

def create():
    return TrackFire()

class TrackFire(bzrobot.AdvancedRobot):

    def run(self):
        self.trackName = ""
        while True:
            try:
                self.turnRight(10)
            except Exception, e:
                print e

    def onScannedRobot(self, scanevent):
        try:
            if(self.trackName == ""):
                self.trackName = scanevent.getName()
            elif(self.trackName != scanevent.getName()):
                return

            self.turnRight(scanevent.getBearing())

            if(scanevent.getDistance() > 30):
                self.ahead(scanevent.getDistance() - 20)
                return
            elif(scanevent.getDistance() < 5):
                self.back(20)
                return

            self.fire()
        except Exception, e:
            print e

    def onRobotDeath(self, deathevent):
        if(deathevent.getName() == self.trackName):
            self.trackName = ""


