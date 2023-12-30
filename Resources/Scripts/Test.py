from EngineLibs import Entity


def OnInit():
    #This will run once, use this to initalize variables and global variables.
    print("This will print once")
    return

def OnUpdate(deltaTime):
    #this will run every frame, use this for your game logic and updating entites
    print("This will print every frame ", deltaTime)
    return