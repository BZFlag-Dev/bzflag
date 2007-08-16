import bzrobot
class TestRobot(bzrobot.BZAdvancedRobot):
    def update():
        print "Hey!"

def create():
    print "create() RULES!"
    return TestRobot()

