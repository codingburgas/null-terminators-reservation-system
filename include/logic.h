#ifndef LOGIC_H
#define LOGIC_H
#include "data.h"

// Алгоритъм за сортиране (Bubble Sort) [cite: 86]
void sortByName();

// Алгоритъм за търсене (Linear Search) [cite: 87]
int findReservationByName(const char* name);

// Рекурсия: Преброяване на общо гости [cite: 88]
int calculateTotalGuestsRecursive(const std::vector<Reservation>& res, int index);
#endif