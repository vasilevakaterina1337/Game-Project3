#pragma once
#include "GameMediator.h"
#include "Handlers.h"
#include <iostream>

class GameFacade {
    std::shared_ptr<GameMediator> mediator;
    std::shared_ptr<Base> base;
    std::shared_ptr<Handler> chain;
    int nextUnitId = 1;
public:
    GameFacade(GameField& field, std::shared_ptr<IObservable> bus, std::shared_ptr<Base> b);
    void processCommand(const std::string& cmd);

    void spawnUnit(int choice, int x, int y);
    void moveUnit(int id, int x, int y);
    void attackUnit(int id, int targetId);
    void interact(int id, int x, int y);
    void printUnits();
    void placeTerrain(int terrainChoice, int x, int y);
    void placeObject(int objectChoice, int x, int y);
};