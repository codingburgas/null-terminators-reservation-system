#include "logic.h"
#include <cstring>

void sortByName() {
    auto& res = getReservations();
    for (size_t i = 0; i < res.size(); i++) {
        for (size_t j = 0; j < res.size() - 1; j++) {
            if (strcmp(res[j].customerName, res[j + 1].customerName) > 0) {
                std::swap(res[j], res[j + 1]);
            }
        }
    }
}

int findReservationByName(const char* name) {
    auto& res = getReservations();
    for (size_t i = 0; i < res.size(); i++) {
        if (strcmp(res[i].customerName, name) == 0) return (int)i;
    }
    return -1;
}

int calculateTotalGuestsRecursive(const std::vector<Reservation>& res, int index) {
    if (index >= res.size()) return 0;
    return res[index].guests + calculateTotalGuestsRecursive(res, index + 1);
}