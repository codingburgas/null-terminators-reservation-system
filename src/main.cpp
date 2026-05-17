#include "data.h"
#include "presentation.h"
#include "auth.h"

int main() {
    initData();
    initAuth();
    runApplication();
    return 0;
}