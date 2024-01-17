import TooGoodEngine


class Entity:
    def __init__(self, name: str):
        self._Entity = TooGoodEngine.PyCreateEntity(name)
    
    def GetName(self):
        name = TooGoodEngine.PyEntityGetName(self._Entity)
        return name
    
    def GetID(self):
        id = TooGoodEngine.PyEntityGetID(self._Entity)
        return id
   

