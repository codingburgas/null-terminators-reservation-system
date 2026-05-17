#ifndef DATA_H
#define DATA_H
#include <vector>
#include <string>

struct Reservation {
    int   id;
    char  customerName[64];
    char  phone[32];         // guest phone number
    int   tableNumber;       // room number
    int   guests;            // number of nights
    char  roomType[64];
    char  packageName[64];
    float totalBill;
    char  checkIn[16];       // "DD/MM/YYYY"
    char  checkOut[16];      // "DD/MM/YYYY"
    char  checkInTime[8];    // "HH:MM"
};

void initData();
std::vector<Reservation>& getReservations();
void addReservation(const Reservation& res);
void deleteReservation(int id);

#endif