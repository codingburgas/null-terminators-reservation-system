#ifndef LOGIC_H
#define LOGIC_H
#include "data.h"

void sortByName();
int findReservationById(int id);
int calculateTotalGuestsRecursive(const std::vector<Reservation>& res, int index);
#endif
