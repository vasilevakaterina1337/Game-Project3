#pragma once
#include "GameMediator.h"
#include "Factories.h"

class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
};

class MoveUnitCommand : public Command {
    GameMediator& mediator;
    int unitId, x, y;
public:
    MoveUnitCommand(GameMediator& m, int id, int nx, int ny);
    void execute() override;
};

class AttackCommand : public Command {
    GameMediator& mediator;
    int attackerId, targetId;
public:
    AttackCommand(GameMediator& m, int atk, int tgt);
    void execute() override;
};

class SpawnUnitCommand : public Command {
    GameMediator& mediator;
    Base& base;
    std::unique_ptr<UnitFactory> factory;
    int x, y;
    int& nextId;
public:
    SpawnUnitCommand(GameMediator& m, Base& b, std::unique_ptr<UnitFactory> f, int nx, int ny, int& idRef);
    void execute() override;
};