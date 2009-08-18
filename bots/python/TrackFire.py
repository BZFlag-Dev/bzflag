# -*- coding: utf-8 -*-
import bzrobot

def create():
    return TrackFire()

class TrackFire(bzrobot.AdvancedRobot):

    def run(self):
        while True:
            try:
                self.turnRight(10)
            except Exception, e:
                print e

    def onScannedRobot(self, scanevent):
        try:
            self.turnRight(scanevent.getBearing())
            self.fire()
        except Exception, e:
            print e


