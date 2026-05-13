#include "data.h"
#include "presentation.h"
#include "auth.h"           // <-- NEW

int main() {
    initData();             // 1. Prepare reservation data
    initAuth();             // 2. Seed the default admin account  <-- NEW
    runApplication();       // 3. Start the GLFW + ImGui loop
    return 0;
}