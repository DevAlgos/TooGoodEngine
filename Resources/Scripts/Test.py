from EngineLibs.Entity import Entity
from EngineLibs.Log import Log, LogWarn, LogError

entity = None


def OnInit():
    entity = Entity("whoo")
    print(entity.GetName())
    
    Log("this is a msg")
    LogWarn("this is a warn")
    LogError("this is a error")

    return


def OnUpdate(deltaTime):
    return



