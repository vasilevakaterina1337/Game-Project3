#include <iostream>
#include <sstream>
#include <memory>
#include <algorithm>
#include <cmath>
#include "GameFacade.h"
#include "Building.h"

// ==========================================
// 1. РЕАЛИЗАЦИЯ МЕТОДОВ GameMediator
// ==========================================

GameMediator::GameMediator(GameField& f, std::shared_ptr<IObservable> bus, Base& b)
    : field(f), eventBus(bus), base(b) {
}

std::shared_ptr<Unit> GameMediator::findUnit(int id) const {
    for (auto& u : allUnits) if (u->getId() == id) return u;
    return nullptr;
}

void GameMediator::addUnit(std::shared_ptr<Unit> unit, int x, int y) {
    if (field.addUnit(unit, x, y)) {
        allUnits.push_back(unit);
        eventBus->notify({ UnitEvent::Created, unit->getId() });
    }
    else {
        std::cout << "[Ошибка] Невозможно разместить юнита на (" << x << "," << y << ").\n";
        base.onUnitEvent({ UnitEvent::Destroyed, unit->getId() });
    }
}

void GameMediator::moveUnit(int unitId, int x, int y) {
    auto u = findUnit(unitId);
    if (!u) { std::cout << "[Ошибка] Юнит не найден.\n"; return; }

    int oldX = u->getX(), oldY = u->getY();
    if (field.getUnitAt(oldX, oldY) == u) field.removeUnit(oldX, oldY);
    if (field.addUnit(u, x, y)) {
        std::cout << "[Медиатор] Юнит " << unitId << " перемещен.\n";
    }
    else {
        field.addUnit(u, oldX, oldY);
        std::cout << "[Ошибка] Невозможно переместить юнита на (" << x << "," << y << ").\n";
    }
}

void GameMediator::attackUnit(int attackerId, int targetId) {
    auto atk = findUnit(attackerId);
    auto tgt = findUnit(targetId);
    if (!atk || !tgt) { std::cout << "[Ошибка] Цель не найдена.\n"; return; }

    int dmg = atk->getAttack();
    tgt->takeDamage(dmg);
    eventBus->notify({ UnitEvent::Damaged, targetId });
    std::cout << "[Медиатор] Юнит " << attackerId << " атаковал " << targetId << " на " << dmg << " урона.\n";

    if (!tgt->isAlive()) {
        if (field.getUnitAt(tgt->getX(), tgt->getY()) == tgt) field.removeUnit(tgt->getX(), tgt->getY());
        eventBus->notify({ UnitEvent::Destroyed, targetId });
        allUnits.erase(std::remove(allUnits.begin(), allUnits.end(), tgt), allUnits.end());
    }
}

void GameMediator::interactWithObject(int unitId, int x, int y) {
    auto obj = field.getObject(x, y);
    auto u = findUnit(unitId);
    if (!obj || !u) { std::cout << "[Ошибка] Объект или юнит не найдены.\n"; return; }
    *obj + *u;
}

void GameMediator::setTerrain(int x, int y, std::shared_ptr<Terrain> t) {
    std::cout << "[DEBUG] Медиатор устанавливает ландшафт на (" << x << "," << y << ")\n";
    field.setTerrain(x, y, t);
}

void GameMediator::setObject(int x, int y, std::shared_ptr<NeutralObject> o) {
    std::cout << "[DEBUG] Медиатор устанавливает объект на (" << x << "," << y << ")\n";
    field.setObject(x, y, o);
}


// ДОБАВЬТЕ ЭТОТ МЕТОД СЮДА:
void GameMediator::setBuilding(int x, int y, std::shared_ptr<Building> b) {
    field.addBuilding(b, x, y);
}
// ==========================================
// 2. РЕАЛИЗАЦИЯ КОМАНД (Commands)
// ==========================================

MoveUnitCommand::MoveUnitCommand(GameMediator& m, int id, int nx, int ny) : mediator(m), unitId(id), x(nx), y(ny) {}
void MoveUnitCommand::execute() { mediator.moveUnit(unitId, x, y); }

AttackCommand::AttackCommand(GameMediator& m, int atk, int tgt) : mediator(m), attackerId(atk), targetId(tgt) {}
void AttackCommand::execute() { mediator.attackUnit(attackerId, targetId); }

SpawnUnitCommand::SpawnUnitCommand(GameMediator& m, Base& b, std::unique_ptr<UnitFactory> f, int nx, int ny, int& idRef)
    : mediator(m), base(b), factory(std::move(f)), x(nx), y(ny), nextId(idRef) {
}

void SpawnUnitCommand::execute() {
    auto unit = factory->createUnit(nextId++);
    base.addUnit(std::shared_ptr<Unit>(unit.release()));
    mediator.addUnit(base.getUnits().back(), x, y);
}

// ==========================================
// 3. РЕАЛИЗАЦИЯ ОБРАБОТЧИКОВ (Handlers)
// ==========================================



// --- Старые ---
bool MoveHandler::handle(const std::string& cmd) {
    if (cmd.find("move") == 0) {
        int id, x, y; std::stringstream ss(cmd); std::string temp;
        ss >> temp >> id >> x >> y; MoveUnitCommand(mediator, id, x, y).execute(); return true;
    }
    return false;
}

bool AttackHandler::handle(const std::string& cmd) {
    if (cmd.find("attack") == 0) {
        int atkId, tgtId; std::stringstream ss(cmd); std::string temp;
        ss >> temp >> atkId >> tgtId; AttackCommand(mediator, atkId, tgtId).execute(); return true;
    }
    return false;
}

bool SpawnHandler::handle(const std::string& cmd) {
    if (cmd.find("spawn") == 0) {
        std::string type; int x, y; std::stringstream ss(cmd); std::string temp;
        ss >> temp >> type >> x >> y;
        std::unique_ptr<UnitFactory> factory;
        if (type == "infantry") factory = std::make_unique<InfantryFactory>();
        else if (type == "archer") factory = std::make_unique<ArcherFactory>();
        else if (type == "cavalry") factory = std::make_unique<CavalryFactory>();
        else if (type == "worker") factory = std::make_unique<WorkerFactory>();
        else if (type == "scout") factory = std::make_unique<ScoutFactory>();
        else if (type == "tank") factory = std::make_unique<TankFactory>();
        else if (type == "medic") factory = std::make_unique<MedicFactory>();
        else return true;
        SpawnUnitCommand(mediator, base, std::move(factory), x, y, nextId).execute();
        return true;
    }
    return false;
}

// --- Новые для ЛР3 ---
bool BuildHandler::handle(const std::string& cmd) {
    if (cmd.find("build") == 0) {
        std::string type; int x, y;
        std::stringstream ss(cmd); std::string temp;

        if (ss >> temp >> type >> x >> y) {
            if (base.wood >= 100) {
                base.wood -= 100;
                auto newBuilding = std::make_shared<Building>(type, x, y);
                base.addBuilding(newBuilding);

                // ДОБАВЬТЕ ЭТУ СТРОКУ:
                mediator.setBuilding(x, y, newBuilding);

                std::cout << "[Система] Начато строительство " << type << " на (" << x << "," << y << ").\n";
            }
            else {
                std::cout << "[Ошибка] Недостаточно дерева!\n";
            }
        }
        else {
            std::cout << "[Ошибка] Неверный формат команды.\n";
        }
        return true;
    }
    return false;
}

bool CollectHandler::handle(const std::string& cmd) {
    if (cmd.find("collect") == 0) {
        int id; std::stringstream ss(cmd); std::string temp;
        ss >> temp >> id;
        auto u = mediator.findUnit(id);
        if (u) {
            if (u->getType() == UnitType::Worker) {
                base.wood += 50;
                std::cout << "[Медиатор] Рабочий " << id << " собрал 50 дерева.\n";
            }
            else {
                std::cout << "[Ошибка] Только Рабочий может собирать ресурсы.\n";
            }
        }
        return true;
    }
    return false;
}

bool HealHandler::handle(const std::string& cmd) {
    if (cmd.find("heal") == 0) {
        int id, targetId; std::stringstream ss(cmd); std::string temp;
        ss >> temp >> id >> targetId;
        auto medic = mediator.findUnit(id);
        auto target = mediator.findUnit(targetId);
        if (medic && target && medic->getType() == UnitType::Medic) {
            target->heal(30);
            std::cout << "[Медиатор] Медик вылечил юнита " << targetId << " на 30 HP.\n";
        }
        return true;
    }
    return false;
}

bool StatsHandler::handle(const std::string& cmd) {
    std::cout << "[DEBUG] StatsHandler получил: '" << cmd << "'\n";
    if (cmd == "stats") {
        base.printStats();
        return true;
    }
    return false;
}


GameFacade::GameFacade(GameField& field, std::shared_ptr<IObservable> bus, std::shared_ptr<Base> b) : base(b) {
    mediator = std::make_shared<GameMediator>(field, bus, *b);
    bus->subscribe(b.get());

    auto moveH = std::make_shared<MoveHandler>(*mediator);
    auto atkH = std::make_shared<AttackHandler>(*mediator);
    auto spawnH = std::make_shared<SpawnHandler>(*mediator, *b, nextUnitId);
    auto buildH = std::make_shared<BuildHandler>(*mediator, *b);
    auto collectH = std::make_shared<CollectHandler>(*mediator, *b);
    auto healH = std::make_shared<HealHandler>(*mediator);
    auto statsH = std::make_shared<StatsHandler>(*b);

    moveH->setNext(atkH);
    atkH->setNext(spawnH);
    spawnH->setNext(buildH); 
    buildH->setNext(collectH);
    collectH->setNext(healH);
    healH->setNext(statsH);

    chain = moveH;
}

void GameFacade::processCommand(const std::string& cmd) {
    std::cout << "[DEBUG] Отправлена команда: '" << cmd << "'\n";

    if (cmd.find("move") == 0) MoveUnitCommand(*mediator, 0, 0, 0).execute();
    else if (cmd.find("stats") == 0) { 
        base->printStats(); 
        return; 
    }
    else if (cmd.find("build") == 0) {
        std::string type; int x, y;
        std::stringstream ss(cmd); std::string temp;
        if (ss >> temp >> type >> x >> y) {
            if (base->wood >= 100) {
                base->wood -= 100;
                auto newBuilding = std::make_shared<Building>(type, x, y);
                base->addBuilding(newBuilding);
                mediator->setBuilding(x, y, newBuilding);
                std::cout << "Начато строительство " << type << " на (" << x << "," << y << ")\n";
            }
            else {
                std::cout << "Недостаточно дерева\n";
            }
            return;
        }
    }

    if (!chain->handle(cmd)) {
        std::cout << "Неизвестная команда\n";
    }
}
void GameFacade::spawnUnit(int choice, int x, int y) {
    std::unique_ptr<UnitFactory> factory;
    switch (choice) {
    case 1: factory = std::make_unique<InfantryFactory>(); 
        break;
    case 2: factory = std::make_unique<ArcherFactory>(); 
        break;
    case 3: factory = std::make_unique<CavalryFactory>(); 
        break;
    case 4: factory = std::make_unique<WorkerFactory>(); 
        break;
    case 5: factory = std::make_unique<ScoutFactory>(); 
        break;
    case 6: factory = std::make_unique<TankFactory>(); 
        break;
    case 7: factory = std::make_unique<MedicFactory>(); 
        break;
    default: return;
    }
    auto unit = factory->createUnit(nextUnitId++);
    base->addUnit(std::shared_ptr<Unit>(unit.release()));
    mediator->addUnit(base->getUnits().back(), x, y);
}

void GameFacade::moveUnit(int id, int x, int y) { 
    mediator->moveUnit(id, x, y); 
}
void GameFacade::attackUnit(int id, int targetId) { 
    mediator->attackUnit(id, targetId); 
}
void GameFacade::interact(int id, int x, int y) { 
    mediator->interactWithObject(id, x, y); 
}

void GameFacade::printUnits() {
    std::cout << "Юниты (всего " << base->getUnitCount() << "):\n";
    for (auto& u : base->getUnits()) {
        std::cout << "  ID: " << u->getId() << " | " << u->getName()
            << " | HP: " << u->getHealth() << " | Поз: (" << u->getX() << "," << u->getY() << ")\n";
    }
}

void GameFacade::placeTerrain(int terrainChoice, int x, int y) {
    std::shared_ptr<Terrain> terrain;
    switch (terrainChoice) {
    case 1: terrain = std::make_shared<Plains>(); std::cout << "Выбрана Равнина\n"; break;
    case 2: terrain = std::make_shared<Forest>(); std::cout << "Выбран Лес\n"; break;
    case 3: terrain = std::make_shared<Swamp>(); std::cout << "Выбрано Болото\n"; break;
    case 4: terrain = std::make_shared<Mountains>(); std::cout << "Выбраны Горы\n"; break;
    default: std::cout << "Неверный выбор\n"; return;
    }
    mediator->setTerrain(x, y, terrain);
}

void GameFacade::placeObject(int objectChoice, int x, int y) {
    std::shared_ptr<NeutralObject> obj;
    switch (objectChoice) {
    case 1: obj = std::make_shared<TreasureChest>(); std::cout << "Поставлен Сундук\n"; break;
    case 2: obj = std::make_shared<Trap>(); std::cout << "Поставлена Ловушка\n"; break;
    case 3: obj = std::make_shared<Altar>(); std::cout << "Поставлен Алтарь\n"; break;
    default: std::cout << "Неверный выбор\n"; return;
    }
    mediator->setObject(x, y, obj);
}

int main() {
    setlocale(LC_ALL, "Russian");

    auto eventBus = std::make_shared<IObservable>();
    auto base = std::make_shared<Base>(1);
    GameField field(10, 10);

    GameFacade game(field, eventBus, base);

    int choice;
    while (true) {
        std::cout << "\nМеню\n"
        << "1. Создать юнита\n"
        << "2. Переместить\n"
        << "3. Атаковать\n"
        << "4. Собрать ресурс (Рабочий)\n"
        << "5. Лечить (Медик)\n"
        << "6. Построить здание\n"
        << "7. Показать карту\n"
        << "8. Показать юнитов\n"
        << "9. Показать статистику\n"
        << "10. Установить ландшафт\n"
        << "11. Установить объект\n"
        << "0. Выход\n"
        << "Выберите действие: ";
        std::cin >> choice;

        if (choice == 0) break;

        int x, y, id, targetId, typeChoice, terrainChoice, objectChoice;

        switch (choice) {
        case 1:
            std::cout << "Тип (1-Пехота, 2-Лучник, 3-Кавалерия, 4-Рабочий, 5-Разведчик, 6-Танк, 7-Медик): ";
            std::cin >> typeChoice;             
            std::cout << "Введите X: ";
            std::cin >> x;
            std::cout << "Введите Y: ";
            std::cin >> y;
            game.spawnUnit(typeChoice, x, y); break;
        case 2:
            std::cout << "Введите ID юнита: ";
            std::cin >> id;
            std::cout << "Введите X: ";
            std::cin >> x;
            std::cout << "Введите Y: ";
            std::cin >> y;
            game.moveUnit(id, x, y);
            break;
        case 3:
            std::cout << "ID атакующего"; 
            std::cin >> id;
            std::cout << "ID цели";
            std::cin >> targetId;
            game.attackUnit(id, targetId); break;
        case 4:
            std::cout << "ID Рабочего: "; 
            std::cin >> id;
            game.processCommand("collect " + std::to_string(id)); 
            break;
        case 5:
            std::cout << "ID Медика и цель: "; std::cin >> id >> targetId;
            game.processCommand("heal " + std::to_string(id) + " " + std::to_string(targetId)); break;
        case 6: { 
            std::cout << "Выберите тип здания:\n";
            std::cout << "1. Казарма (barracks)\n";
            std::cout << "2. Лаборатория (lab)\n";
            std::cout << "3. Склад (warehouse)\n";
            std::cout << "Ваш выбор: ";
            std::cin >> typeChoice;

            std::string buildingType;
            if (typeChoice == 1) buildingType = "barracks";
            else if (typeChoice == 2) buildingType = "lab";
            else if (typeChoice == 3) buildingType = "warehouse";
            else { std::cout << "Неверный тип здания\n"; break; }

            std::cout << "Введите X: "; std::cin >> x;
            std::cout << "Введите Y: "; std::cin >> y;

            game.processCommand("build " + buildingType + " " + std::to_string(x) + " " + std::to_string(y));
            break;
        }
        case 7:
            field.printMap();
            break;
        case 8:
            game.printUnits();
            break;
        case 9:
            game.processCommand("stats");
            break;
        case 10:
            std::cout << "Ландшафт (1-Равнина, 2-Лес, 3-Болото, 4-Горы): ";
            std::cin >> terrainChoice;
            std::cout << "Введите X: ";
            std::cin >> x;
            std::cout << "Введите Y: ";
            std::cin >> y;
            game.placeTerrain(terrainChoice, x, y);
            break;
        case 11:
            std::cout << "Объект (1-Сундук, 2-Ловушка, 3-Алтарь): ";
            std::cin >> objectChoice;
            std::cout << "Введите X: ";
            std::cin >> x;
            std::cout << "Введите Y: ";
            std::cin >> y;
            game.placeObject(objectChoice, x, y);
            break;
        default:
            std::cout << "Неверный ввод\n";
        }
    }

    std::cout << "Пока-пока\n";
    return 0;
}