import TGE

class Entity:
    def __init__(self, name):
        self._Entity = TGE.PyCreateEntity(name)
    
    def GetName(self):
        name = TGE.PyEntityGetName(self._Entity)
        return name
    
    def GetID(self):
        id = TGE.PyEntityGetID(self._Entity)
        return id
