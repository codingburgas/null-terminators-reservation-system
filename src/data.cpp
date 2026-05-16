#include "data.h"

static std::vector<Reservation> database;

void initData() {
    database.push_back({ 1, "Vanko", 5, 4 });
    database.push_back({ 2, "Ivan", 2, 2 });
}

std::vector<Reservation>& getReservations() {
    return database;
}

void addReservation(const Reservation& res) {
    database.push_back(res);
}

void deleteReservation(int id) {
    auto& db = database;
    for (auto it = db.begin(); it != db.end(); ++it) {
        if (it->id == id) {
            db.erase(it);
            return;
        }
    }
}
