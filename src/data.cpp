#define _CRT_SECURE_NO_WARNINGS
#include "data.h"
#include <cstring>
#include <ctime>

static std::vector<Reservation> database;

static void formatDate(time_t t, char* buf, size_t size) {
    struct tm ts;
    localtime_s(&ts, &t);
    strftime(buf, size, "%d/%m/%Y", &ts);
}

void initData() {
    time_t now = time(nullptr);
    const time_t DAY = 86400;

    // Reservation 1 – checked in yesterday, 3 nights
    {
        Reservation r{};
        r.id = 1;
        strncpy_s(r.customerName, "Vanko Ivanov (vanko@mail.com)", sizeof(r.customerName) - 1);
        strncpy_s(r.phone, "+359 88 123 4567", sizeof(r.phone) - 1);
        r.tableNumber = 101;
        r.guests = 3;
        strncpy_s(r.roomType, "Deluxe Room", sizeof(r.roomType) - 1);
        strncpy_s(r.packageName, "All Incl.", sizeof(r.packageName) - 1);
        r.totalBill = 600.0f;
        formatDate(now - DAY, r.checkIn, sizeof(r.checkIn));
        formatDate(now - DAY + 3 * DAY, r.checkOut, sizeof(r.checkOut));
        strncpy_s(r.checkInTime, "14:00", sizeof(r.checkInTime) - 1);
        database.push_back(r);
    }

    // Reservation 2 – checking in today, 2 nights
    {
        Reservation r{};
        r.id = 2;
        strncpy_s(r.customerName, "Ivan Petrov (ivan@mail.com)", sizeof(r.customerName) - 1);
        strncpy_s(r.phone, "+359 87 765 4321", sizeof(r.phone) - 1);
        r.tableNumber = 205;
        r.guests = 2;
        strncpy_s(r.roomType, "Standard Room", sizeof(r.roomType) - 1);
        strncpy_s(r.packageName, "Bed & Breakfast", sizeof(r.packageName) - 1);
        r.totalBill = 200.0f;
        formatDate(now, r.checkIn, sizeof(r.checkIn));
        formatDate(now + 2 * DAY, r.checkOut, sizeof(r.checkOut));
        strncpy_s(r.checkInTime, "12:00", sizeof(r.checkInTime) - 1);
        database.push_back(r);
    }

    // Reservation 3 – Presidential Suite, checked in 2 days ago, 7 nights
    {
        Reservation r{};
        r.id = 3;
        strncpy_s(r.customerName, "Sofia Marchetti (sofia.m@luxmail.it)", sizeof(r.customerName) - 1);
        strncpy_s(r.phone, "+39 02 9988 7766", sizeof(r.phone) - 1);
        r.tableNumber = 501;
        r.guests = 7;
        strncpy_s(r.roomType, "Presidential Suite", sizeof(r.roomType) - 1);
        strncpy_s(r.packageName, "All Incl.", sizeof(r.packageName) - 1);
        r.totalBill = 3500.0f;
        formatDate(now - 2 * DAY, r.checkIn, sizeof(r.checkIn));
        formatDate(now - 2 * DAY + 7 * DAY, r.checkOut, sizeof(r.checkOut));
        strncpy_s(r.checkInTime, "15:00", sizeof(r.checkInTime) - 1);
        database.push_back(r);
    }

    // Reservation 4 – Standard Room, arriving tomorrow, 4 nights
    {
        Reservation r{};
        r.id = 4;
        strncpy_s(r.customerName, "Georgi Nikolov (g.nikolov@abv.bg)", sizeof(r.customerName) - 1);
        strncpy_s(r.phone, "+359 89 234 5678", sizeof(r.phone) - 1);
        r.tableNumber = 302;
        r.guests = 4;
        strncpy_s(r.roomType, "Standard Room", sizeof(r.roomType) - 1);
        strncpy_s(r.packageName, "Room Only", sizeof(r.packageName) - 1);
        r.totalBill = 400.0f;
        formatDate(now + DAY, r.checkIn, sizeof(r.checkIn));
        formatDate(now + DAY + 4 * DAY, r.checkOut, sizeof(r.checkOut));
        strncpy_s(r.checkInTime, "13:30", sizeof(r.checkInTime) - 1);
        database.push_back(r);
    }

    // Reservation 5 – Deluxe Room, arriving in 3 days, 5 nights
    {
        Reservation r{};
        r.id = 5;
        strncpy_s(r.customerName, "Amir Hassan (amir.h@outlook.com)", sizeof(r.customerName) - 1);
        strncpy_s(r.phone, "+971 50 444 9900", sizeof(r.phone) - 1);
        r.tableNumber = 401;
        r.guests = 5;
        strncpy_s(r.roomType, "Deluxe Room", sizeof(r.roomType) - 1);
        strncpy_s(r.packageName, "Half Board", sizeof(r.packageName) - 1);
        r.totalBill = 1000.0f;
        formatDate(now + 3 * DAY, r.checkIn, sizeof(r.checkIn));
        formatDate(now + 3 * DAY + 5 * DAY, r.checkOut, sizeof(r.checkOut));
        strncpy_s(r.checkInTime, "16:00", sizeof(r.checkInTime) - 1);
        database.push_back(r);
    }

    // Reservation 6 – Suite, checked in 4 days ago, leaving tomorrow
    {
        Reservation r{};
        r.id = 6;
        strncpy_s(r.customerName, "Elena Popova (e.popova@gmail.com)", sizeof(r.customerName) - 1);
        strncpy_s(r.phone, "+7 916 321 0099", sizeof(r.phone) - 1);
        r.tableNumber = 401;
        r.guests = 5;
        strncpy_s(r.roomType, "Suite", sizeof(r.roomType) - 1);
        strncpy_s(r.packageName, "Bed & Breakfast", sizeof(r.packageName) - 1);
        r.totalBill = 750.0f;
        formatDate(now - 4 * DAY, r.checkIn, sizeof(r.checkIn));
        formatDate(now + DAY, r.checkOut, sizeof(r.checkOut));
        strncpy_s(r.checkInTime, "14:00", sizeof(r.checkInTime) - 1);
        database.push_back(r);
    }

    // Reservation 7 – Standard Room, arriving in 6 days, 2 nights
    {
        Reservation r{};
        r.id = 7;
        strncpy_s(r.customerName, "Lucas Ferreira (lferreira@proton.me)", sizeof(r.customerName) - 1);
        strncpy_s(r.phone, "+55 11 98765 4321", sizeof(r.phone) - 1);
        r.tableNumber = 202;
        r.guests = 2;
        strncpy_s(r.roomType, "Standard Room", sizeof(r.roomType) - 1);
        strncpy_s(r.packageName, "Room Only", sizeof(r.packageName) - 1);
        r.totalBill = 200.0f;
        formatDate(now + 6 * DAY, r.checkIn, sizeof(r.checkIn));
        formatDate(now + 6 * DAY + 2 * DAY, r.checkOut, sizeof(r.checkOut));
        strncpy_s(r.checkInTime, "11:00", sizeof(r.checkInTime) - 1);
        database.push_back(r);
    }

    // Reservation 8 – Deluxe Room, today, 1 night
    {
        Reservation r{};
        r.id = 8;
        strncpy_s(r.customerName, "Yui Tanaka (yui.tanaka@jpmail.jp)", sizeof(r.customerName) - 1);
        strncpy_s(r.phone, "+81 90 1234 5678", sizeof(r.phone) - 1);
        r.tableNumber = 203;
        r.guests = 1;
        strncpy_s(r.roomType, "Deluxe Room", sizeof(r.roomType) - 1);
        strncpy_s(r.packageName, "Room Only", sizeof(r.packageName) - 1);
        r.totalBill = 200.0f;
        formatDate(now, r.checkIn, sizeof(r.checkIn));
        formatDate(now + DAY, r.checkOut, sizeof(r.checkOut));
        strncpy_s(r.checkInTime, "09:00", sizeof(r.checkInTime) - 1);
        database.push_back(r);
    }

    // Reservation 9 – Presidential Suite, arriving in 10 days, 3 nights
    {
        Reservation r{};
        r.id = 9;
        strncpy_s(r.customerName, "Marcus Webb (m.webb@corporateinc.com)", sizeof(r.customerName) - 1);
        strncpy_s(r.phone, "+1 212 555 0198", sizeof(r.phone) - 1);
        r.tableNumber = 501;
        r.guests = 3;
        strncpy_s(r.roomType, "Presidential Suite", sizeof(r.roomType) - 1);
        strncpy_s(r.packageName, "All Incl.", sizeof(r.packageName) - 1);
        r.totalBill = 1500.0f;
        formatDate(now + 10 * DAY, r.checkIn, sizeof(r.checkIn));
        formatDate(now + 10 * DAY + 3 * DAY, r.checkOut, sizeof(r.checkOut));
        strncpy_s(r.checkInTime, "18:00", sizeof(r.checkInTime) - 1);
        database.push_back(r);
    }
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