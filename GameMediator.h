#pragma once
#include "GameField.h"
#include "Base.h"
#include <vector>
#include <algorithm>
#include "Building.h"
class GameMediator {
    GameField& field;
    Base& base;
    std::vector<std::shared_ptr<Unit>> allUnits;
    std::shared_ptr<IObservable> eventBus;
public:
    void setBuilding(int x, int y, std::shared_ptr<Building> b);
    GameMediator(GameField& f, std::shared_ptr<IObservable> bus, Base& b);

    void addUnit(std::shared_ptr<Unit> unit, int x, int y);
    void moveUnit(int unitId, int x, int y);
    void attackUnit(int attackerId, int targetId);
    void interactWithObject(int unitId, int x, int y);

    void setTerrain(int x, int y, std::shared_ptr<Terrain> t);
    void setObject(int x, int y, std::shared_ptr<NeutralObject> o);

    std::shared_ptr<Unit> findUnit(int id) const;
};