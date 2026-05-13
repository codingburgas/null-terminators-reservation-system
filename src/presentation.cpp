#include "presentation.h"
#include "auth.h"           // <-- NEW
#include "imgui.h"
#include "logic.h"
#include <cstdio>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

void renderUI() {
    static char searchBuf[64] = "";
    static int searchResultIdx = -2;
    auto& reservations = getReservations();

    // Show the logged-in user in the title bar
    AuthSession& session = getAuthSession();
    char title[128];
    snprintf(title, sizeof(title), "Reservation Management System  |  %s %s (%s)",
        session.currentUser.firstName.c_str(),
        session.currentUser.lastName.c_str(),
        session.currentUser.role == UserRole::Admin ? "Admin" : "Guest");

    ImGui::Begin(title);

    if (ImGui::Button("Sort by Name")) {
        sortByName();
    }

    ImGui::InputText("Search Customer", searchBuf, 64);
    if (ImGui::Button("Find")) {
        searchResultIdx = findReservationByName(searchBuf);
    }

    int total = calculateTotalGuestsRecursive(reservations, 0);
    ImGui::Text("Total Guests (Calculated via Recursion): %d", total);

    ImGui::Separator();

    if (ImGui::BeginTable("ResTable", 4, ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Table");
        ImGui::TableSetupColumn("Guests");
        ImGui::TableHeadersRow();

        for (int i = 0; i < (int)reservations.size(); i++) {
            ImGui::TableNextRow();
            if (i == searchResultIdx)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
                    ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.5f)));

            ImGui::TableSetColumnIndex(0); ImGui::Text("%d", reservations[i].id);
            ImGui::TableSetColumnIndex(1); ImGui::Text("%s", reservations[i].customerName);
            ImGui::TableSetColumnIndex(2); ImGui::Text("%d", reservations[i].tableNumber);
            ImGui::TableSetColumnIndex(3); ImGui::Text("%d", reservations[i].guests);
        }
        ImGui::EndTable();
    }

    ImGui::End();
}

void runApplication() {
    if (!glfwInit()) return;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Reservation System", NULL, NULL);
    glfwMakeContextCurrent(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ── AUTH GATE ──────────────────────────────────────────────────────
        // Show the login/register screen until the user is authenticated.
        // Once renderAuthUI() returns true the session is marked loggedIn
        // and we switch to the main app UI permanently.
        if (!getAuthSession().loggedIn) {
            renderAuthUI();     // <-- NEW: show login / register
        }
        else {
            renderUI();         // existing reservation UI
        }
        // ──────────────────────────────────────────────────────────────────

        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}