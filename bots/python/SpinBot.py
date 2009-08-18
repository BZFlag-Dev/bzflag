# -*- coding: utf-8 -*-
import bzrobot

def create():
    return SpinBot()

class SpinBot(bzrobot.AdvancedRobot):

    def run(self):
        while True:
            try:
                self.doNothing()
                self.setTurnRight(10000)
                self.setMaxVelocity(5)
                self.ahead(10000)
            except Exception, e:
                print e

    def onScannedRobot(self, scanevent):
        try:
            if(abs(scanevent.getBearing()) < 1):
                self.fire()
        except Exception, e:
            print e


