#pragma once
#include <string>
#include <memory>
#include <sstream>
#include <iostream>
#include "Commands.h"
#include "Factories.h"
#include "Building.h"

class Handler {
protected:
    std::shared_ptr<Handler> next;
public:
    void setNext(std::shared_ptr<Handler> n) { 
        next = n; 
    }

    virtual bool handle(const std::string& cmd) = 0;
    virtual ~Handler() = default;
};

class MoveHandler : public Handler {
    GameMediator& mediator;
public:
    MoveHandler(GameMediator& m) : mediator(m) {}
    bool handle(const std::string& cmd) override;
};

class AttackHandler : public Handler {
    GameMediator& mediator;
public:
    AttackHandler(GameMediator& m) : mediator(m) {}
    bool handle(const std::string& cmd) override;
};

class SpawnHandler : public Handler {
    GameMediator& mediator;
    Base& base;
    int& nextId;
public:
    SpawnHandler(GameMediator& m, Base& b, int& idRef) : mediator(m), base(b), nextId(idRef) {}
    bool handle(const std::string& cmd) override;
};

class BuildHandler : public Handler {
    GameMediator& mediator;
    Base& base;
public:
    BuildHandler(GameMediator& m, Base& b) : mediator(m), base(b) {}
    bool handle(const std::string& cmd) override;
};

class CollectHandler : public Handler {
    GameMediator& mediator;
    Base& base;
public:
    CollectHandler(GameMediator& m, Base& b) : mediator(m), base(b) {}
    bool handle(const std::string& cmd) override;
};

class HealHandler : public Handler {
    GameMediator& mediator;
public:
    HealHandler(GameMediator& m) : mediator(m) {}
    bool handle(const std::string& cmd) override;
};

class StatsHandler : public Handler {
    Base& base;
public:
    StatsHandler(Base& b) : base(b) {}
    bool handle(const std::string& cmd) 
        override;
};