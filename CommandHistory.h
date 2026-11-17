#pragma once
#include <vector>
#include <memory>
#include "Commands.h"

class CommandHistory {
    std::vector<std::shared_ptr<Command>> history;
public:
    void push(std::shared_ptr<Command> cmd) { 
        history.push_back(cmd); 
    }
    void undo() { 
        if (!history.empty()) 
            history.pop_back(); 
    }
};
class CollectResourceCommand : public Command {
    GameMediator& mediator;
    int unitId; int x, y;
public:
    CollectResourceCommand(GameMediator& m, int id, int x, int y) : mediator(m), unitId(id), x(x), y(y) {}
    void execute() override {}
};

class BuildBuildingCommand : public Command {
    GameMediator& mediator;
    std::string type; int x, y;
public:
    BuildBuildingCommand(GameMediator& m, std::string t, int x, int y) : mediator(m), type(t), x(x), y(y) {}
    void execute() override {}
};

class StatsCommand : public Command {
    GameMediator& mediator;
public:
    StatsCommand(GameMediator& m) : mediator(m) {}
    void execute() override {}
};