import bzrobot
class TestRobot(bzrobot.BZAdvancedRobot):
    def update(self):
        print "Hey!"

def create():
    print "create() RULES!"
    return TestRobot()

