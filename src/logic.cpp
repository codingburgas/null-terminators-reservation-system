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

int findReservationById(int id) {
    auto& res = getReservations();
    for (size_t i = 0; i < res.size(); i++) {
        if (res[i].id == id) return (int)i;
    }
    return -1;
}

int calculateTotalGuestsRecursive(const std::vector<Reservation>& res, int index) {
    if (index >= (int)res.size()) return 0;
    return res[index].guests + calculateTotalGuestsRecursive(res, index + 1);
}
