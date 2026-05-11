#ifndef DATA_H
#define DATA_H
#include <vector>
#include <string>

struct Reservation {
    int id;
    char customerName[64];
    int tableNumber;
    int guests;
};

// Прототипи на функции [cite: 135]
void initData();
std::vector<Reservation>& getReservations();
void addReservation(const Reservation& res);
#endif