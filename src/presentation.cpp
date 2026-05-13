#include "presentation.h"
#include "auth.h"           // <-- NEW
#include "imgui.h"
#include "logic.h"
#include <cstdio>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

void setupModernTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f); 
	style.Colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.35f, 1.00f); // accent color for headers
    style.Colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.40f, 0.65f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);

    style.WindowRounding = 10.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.ItemSpacing = ImVec2(10, 10);
}

void renderUI() {
    setupModernTheme();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

	// Clear window flags for a clean, full-screen dashboard look
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    static char searchBuf[64] = "";
    static int searchResultIdx = -2;
    auto& reservations = getReservations();

    ImGui::Begin("Hotel Management System", nullptr, window_flags);

    // --- HEADER ---
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 0.84f, 0.00f, 1.00f));
    ImGui::SetWindowFontScale(1.4f);
    ImGui::Text("HOTEL NullTerminators - EXECUTIVE MANAGEMENT");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();


    AuthSession& session = getAuthSession();

    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1.0f), "System Version 2.1 | Logged in as: %s", session.currentUser.role == UserRole::Admin ? "Admin" : "Guest");
    ImGui::Separator();
    ImGui::Spacing();

	// --- Stats upper card ---
    ImGui::Columns(3, "stats", false);

    ImGui::BeginChild("Stat1", ImVec2(0, 70), true);
    ImGui::Text("TOTAL RESERVATIONS");
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%zu", reservations.size());
    ImGui::EndChild();

    ImGui::NextColumn();
    ImGui::BeginChild("Stat2", ImVec2(0, 70), true);
    ImGui::Text("GUESTS IN HOUSE");
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%d", calculateTotalGuestsRecursive(reservations, 0));
    ImGui::EndChild();

    ImGui::NextColumn();
    ImGui::BeginChild("Stat3", ImVec2(0, 70), true);
    ImGui::Text("AVAILABLE ROOMS");
    
    int available = 500 - (int)reservations.size();
    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%d / 500", available);
    ImGui::EndChild();

    ImGui::Columns(1);
    ImGui::Spacing();

    // --- Main Pannel ---
    ImGui::Columns(2, "mainLayout", true);
	ImGui::SetColumnWidth(0, 300); // Left column for quick actions and search

	// Left: Quick actions + search
    ImGui::Text("QUICK ACTIONS");
    if (ImGui::Button("Sort Guest List", ImVec2(-1, 40))) {
        sortByName();
    }

    ImGui::Spacing();
    ImGui::Text("Search Guest:");

    
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 12));
    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##search", "Name...", searchBuf, 64);
    ImGui::PopStyleVar();

    ImGui::Spacing();
    if (ImGui::Button("FIND", ImVec2(-1, 45))) {
        searchResultIdx = findReservationByName(searchBuf);
    }

    if (ImGui::Button("RESET", ImVec2(-1, 35))) {
        searchResultIdx = -2;
        searchBuf[0] = '\0';
    }

    ImGui::NextColumn();

	// Rigth: Table with reservations
    ImGui::Text("CURRENT OCCUPANCY");

	// enlarge font for better readability of the table
    ImGui::SetWindowFontScale(1.1f);

    // More columns (7 in total)
    if (ImGui::BeginTable("GuestTable", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, -10))) {
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 35.0f);
        ImGui::TableSetupColumn("Customer Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Room", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Period", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Package", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Bill", ImGuiTableColumnFlags_WidthFixed, 80.0f);

        ImGui::TableHeadersRow();

        for (int i = 0; i < reservations.size(); i++) {
			ImGui::TableNextRow(0, 32.0f); // Higher row height for better readability

            if (i == searchResultIdx)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(50, 150, 50, 180));

            ImGui::TableSetColumnIndex(0); ImGui::Text("%d", reservations[i].id);
            ImGui::TableSetColumnIndex(1); ImGui::Text("%s", reservations[i].customerName);
            ImGui::TableSetColumnIndex(2); ImGui::Text("%d", reservations[i].tableNumber);

			// New columns with dummy data for demonstration
			ImGui::TableSetColumnIndex(3); ImGui::Text("%d nights", reservations[i].guests); // Period of stay
            ImGui::TableSetColumnIndex(4); ImGui::Text("Deluxe");   // Type room
			ImGui::TableSetColumnIndex(5); ImGui::Text("All Incl."); // Type of package
            ImGui::TableSetColumnIndex(6); ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.00f), "$ %.2f", (float)reservations[i].guests * 150.0f); // Bill
        }
        ImGui::EndTable();
    }
	ImGui::SetWindowFontScale(1.0f); // Return to normal font scale

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