#ifndef DATA_H
#define DATA_H
#include <vector>
#include <string>

struct Reservation {
    int id;
    char customerName[64];
    int tableNumber;
    int guests;
    char roomType[64];
    char packageName[64];
    float totalBill;
};

void initData();
std::vector<Reservation>& getReservations();
void addReservation(const Reservation& res);
void deleteReservation(int id);
#endif
