#include "presentation.h"
#include "auth.h"
#include "data.h"
#include "imgui.h"
#include "logic.h"
#include <cstdio>
#include <cstdarg>
#define _CRT_SECURE_NO_WARNINGS
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <ctime>
#include <string>

void setupModernTheme() {
    ImGuiStyle& style = ImGui::GetStyle();

    // --- ЗАОБЛЯНЕ (Modern Rounding) ---
    style.WindowRounding = 0.0f;     // Основният прозорец е на цял екран
    style.ChildRounding = 10.0f;    // Заоблени кутии за статитстика
    style.FrameRounding = 8.0f;     // Заоблени бутони и полета за търсене
    style.PopupRounding = 8.0f;
    style.GrabRounding = 8.0f;
    style.ItemSpacing = ImVec2(12, 12);

    // --- ЦВЕТОВА ПАЛИТРА (Deep Dark & Blue Accents) ---
    ImVec4* colors = style.Colors;

    // По-тъмен и модерен фон
    colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.04f, 0.05f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);

    // Синьо за контури и рамки (Borders)
    colors[ImGuiCol_Border] = ImVec4(0.15f, 0.25f, 0.45f, 0.60f);
    colors[ImGuiCol_Separator] = ImVec4(0.15f, 0.25f, 0.45f, 0.60f);

    // Бутони (Елегантно синьо)
    colors[ImGuiCol_Button] = ImVec4(0.12f, 0.18f, 0.30f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.35f, 0.60f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.08f, 0.12f, 0.20f, 1.00f);

    // Полета за въвеждане
    colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
}

void renderUI() {
    setupModernTheme();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    static int  searchResultIdx = -2;
    static int  searchId        = 0;
    static int  deleteTargetId  = -1;
    static char deleteTargetName[64] = "";
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
    ImGui::SameLine();
    float logoffWidth = 90.0f;
    ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - logoffWidth);
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.35f, 0.08f, 0.08f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.60f, 0.12f, 0.12f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.20f, 0.05f, 0.05f, 1.00f));
    if (ImGui::Button("Log Off", ImVec2(logoffWidth, 0))) {
        logoutAuth();
    }
    ImGui::PopStyleColor(3);

    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.15f, 0.25f, 0.45f, 0.60f));
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // --- Stats upper cards ---
    ImGui::Columns(3, "stats", false);

    ImGui::BeginChild("Stat1", ImVec2(0, 80), true, ImGuiWindowFlags_None);
    ImGui::Text("TOTAL RESERVATIONS");
    ImGui::SetWindowFontScale(1.9f);
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%zu", reservations.size());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::EndChild();

    ImGui::NextColumn();
    ImGui::BeginChild("Stat2", ImVec2(0, 80), true, ImGuiWindowFlags_None);
    ImGui::Text("GUESTS IN HOUSE");
    ImGui::SetWindowFontScale(1.9f);
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%d", calculateTotalGuestsRecursive(reservations, 0));
    ImGui::SetWindowFontScale(1.0f);
    ImGui::EndChild();

    ImGui::NextColumn();
    ImGui::BeginChild("Stat3", ImVec2(0, 80), true, ImGuiWindowFlags_None);
    ImGui::Text("AVAILABLE ROOMS");
    int available = 500 - (int)reservations.size();
    ImGui::SetWindowFontScale(1.9f);
    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%d / 500", available);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::EndChild();

    ImGui::Columns(1);
    ImGui::Spacing();

    // --- Main Panel ---
    ImGui::Columns(2, "mainLayout", true);
    ImGui::SetColumnWidth(0, 300);

    // Left: Quick actions + search by ID
    ImGui::Text("QUICK ACTIONS");
    if (ImGui::Button("Sort Guest List", ImVec2(-1, 40))) {
        sortByName();
    }

    ImGui::Spacing();
    ImGui::Text("Search by ID:");
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 12));
    ImGui::SetNextItemWidth(-1);
    ImGui::InputInt("##searchId", &searchId, 0, 0);
    if (searchId < 0) searchId = 0;
    ImGui::PopStyleVar();

    ImGui::Spacing();
    if (ImGui::Button("FIND", ImVec2(-1, 45))) {
        searchResultIdx = findReservationById(searchId);
    }
    if (ImGui::Button("RESET", ImVec2(-1, 35))) {
        searchResultIdx = -2;
        searchId = 0;
    }

    ImGui::NextColumn();

    // Right: Table with reservations
    ImGui::Text("CURRENT OCCUPANCY");
    ImGui::SetWindowFontScale(1.1f);

    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.25f, 0.45f, 0.30f));
    if (ImGui::BeginTable("GuestTable", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, -10))) {
        ImGui::TableSetupColumn("ID",            ImGuiTableColumnFlags_WidthFixed,   35.0f);
        ImGui::TableSetupColumn("Customer Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Room",          ImGuiTableColumnFlags_WidthFixed,   50.0f);
        ImGui::TableSetupColumn("Period",        ImGuiTableColumnFlags_WidthFixed,   70.0f);
        ImGui::TableSetupColumn("Type",          ImGuiTableColumnFlags_WidthFixed,   80.0f);
        ImGui::TableSetupColumn("Package",       ImGuiTableColumnFlags_WidthFixed,   80.0f);
        ImGui::TableSetupColumn("Bill",          ImGuiTableColumnFlags_WidthFixed,   80.0f);
        ImGui::TableSetupColumn("Action",        ImGuiTableColumnFlags_WidthFixed,   60.0f);
        ImGui::TableHeadersRow();

        for (int i = 0; i < (int)reservations.size(); i++) {
            ImGui::TableNextRow(0, 32.0f);

            if (i == searchResultIdx)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(50, 150, 50, 180));

            ImGui::TableSetColumnIndex(0); ImGui::Text("%d", reservations[i].id);
            ImGui::TableSetColumnIndex(1); ImGui::Text("%s", reservations[i].customerName);
            ImGui::TableSetColumnIndex(2); ImGui::Text("%d", reservations[i].tableNumber);
            ImGui::TableSetColumnIndex(3); ImGui::Text("%d nights", reservations[i].guests);
            ImGui::TableSetColumnIndex(4); ImGui::Text("Deluxe");
            ImGui::TableSetColumnIndex(5); ImGui::Text("All Incl.");
            ImGui::TableSetColumnIndex(6); ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.00f), "$ %.2f", (float)reservations[i].guests * 150.0f);

            ImGui::TableSetColumnIndex(7);
            ImGui::PushID(i);
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.35f, 0.08f, 0.08f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.60f, 0.12f, 0.12f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.20f, 0.05f, 0.05f, 1.0f));
            if (ImGui::SmallButton("Delete")) {
                deleteTargetId = reservations[i].id;
                strncpy_s(deleteTargetName, reservations[i].customerName, sizeof(deleteTargetName) - 1);
                ImGui::OpenPopup("Confirm Delete");
            }
            ImGui::PopStyleColor(3);
            ImGui::PopID();
        }

        // ── Delete confirmation modal ──────────────────────────────────────
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(340, 0), ImGuiCond_Always);
        if (ImGui::BeginPopupModal("Confirm Delete", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            ImGui::Spacing();
            ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Delete Reservation?").x) * 0.5f);
            ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.0f), "Delete Reservation?");
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("Guest:"); ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s", deleteTargetName);
            ImGui::Spacing();
            ImGui::TextDisabled("This action cannot be undone.");
            ImGui::Spacing();

            float btnW = (ImGui::GetContentRegionAvail().x - 8.0f) * 0.5f;

            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.35f, 0.08f, 0.08f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.60f, 0.12f, 0.12f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.20f, 0.05f, 0.05f, 1.0f));
            if (ImGui::Button("Delete", ImVec2(btnW, 36))) {
                deleteReservation(deleteTargetId);
                if (searchResultIdx >= 0) searchResultIdx = -2;
                deleteTargetId = -1;
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopStyleColor(3);

            ImGui::SameLine(0, 8);
            if (ImGui::Button("Cancel", ImVec2(btnW, 36))) {
                deleteTargetId = -1;
                ImGui::CloseCurrentPopup();
            }
            ImGui::Spacing();
            ImGui::EndPopup();
        }
        // ──────────────────────────────────────────────────────────────────

        ImGui::EndTable();
    }
    ImGui::PopStyleColor();
    ImGui::SetWindowFontScale(1.0f);

    ImGui::End();
}

void renderGuestUI() {
    setupModernTheme();


    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    
    static char gName[64] = "", gPhone[32] = "", gEmail[64] = "";
    static int gDays = 1, gGuests = 1, gRooms = 1, gBeds = 1;
    static int roomType = 0; // 0: Standard, 1: Deluxe, 2: Suite
    const char* roomTypes[] = { "Standard Room", "Deluxe Room", "Presidential Suite" };
    float roomPrices[] = { 100.0f, 200.0f, 500.0f };

    time_t t = time(0);
    struct tm now_struct;
    localtime_s(&now_struct, &t);
    struct tm* now = &now_struct;

    // 
    time_t endT = t + (static_cast<long long>(gDays) * 24 * 60 * 60);
    struct tm endNow_struct;
    localtime_s(&endNow_struct, &endT);
    struct tm* endNow = &endNow_struct;

    static char startDate[16] = "";
    static char endDate[16] = "";
    static bool datesInitialized = false;

    if (!datesInitialized) {
        strftime(startDate, sizeof(startDate), "%d/%m/%Y", now);
        datesInitialized = true;
    }

    strftime(endDate, sizeof(endDate), "%d/%m/%Y", endNow);

    static bool useSpecificDates = false;

    auto& reservations = getReservations();
    AuthSession& session = getAuthSession();

    ImGui::Begin("Guest Portal", nullptr, window_flags);

    // --- HEADER ---
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 0.84f, 0.00f, 1.00f));
    ImGui::SetWindowFontScale(1.4f);
    ImGui::Text("HOTEL NullTerminators - GUEST PORTAL");
    ImGui::PopStyleColor();
    ImGui::SetWindowFontScale(1.0f);

    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Status: Online");
    ImGui::Separator();
    ImGui::Spacing();

    // --- TOP STATS (3 Columns) ---
    ImGui::Columns(3, "guestStats", false);

	// 1. Guest name
    ImGui::BeginChild("Stat1", ImVec2(0, 80), true);
    ImGui::Text("WELCOME,");
    ImGui::SetWindowFontScale(1.3f);
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", session.currentUser.firstName.c_str());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::EndChild();

    ImGui::NextColumn();

	// 2.Free rooms
    ImGui::BeginChild("Stat2", ImVec2(0, 80), true);
    ImGui::Text("AVAILABLE SLOTS");
    int available = 500 - (int)reservations.size();
    ImGui::SetWindowFontScale(1.8f);
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%d", available);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::EndChild();

    ImGui::NextColumn();

    
    ImGui::BeginChild("Stat3", ImVec2(0, 80), true);
    ImGui::Text("TODAY'S DATE");
    ImGui::SetWindowFontScale(1.5f);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", startDate); 
    ImGui::SetWindowFontScale(1.0f);
    ImGui::EndChild();

    ImGui::Columns(1);
    ImGui::Spacing();

    // --- BOOKING FORM ---
    ImGui::Text("MAKE A NEW RESERVATION");
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.12f, 1.00f));
    ImGui::BeginChild("BookingForm", ImVec2(0, 0), true);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Columns(2, "formSplit", false);

	// Personal info on the left
    ImGui::Text("Personal Information");
    ImGui::InputTextWithHint("##name", "Full Name", gName, 64);
    ImGui::InputTextWithHint("##phone", "Phone Number", gPhone, 32);
    ImGui::InputTextWithHint("##email", "Email Address", gEmail, 64);

    ImGui::Text("Dates Selection");

    // 1. Checkbox for specific dates

    if (useSpecificDates) {
        ImGui::InputText("Check-in Date", startDate, 16);
    }
    else {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Check-in: %s (Today)", startDate);
    }

    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Check-out: %s", endDate);

    ImGui::NextColumn();

	// Right column: Stay details
    ImGui::Text("Stay Details");
    ImGui::Combo("Room Type", &roomType, roomTypes, IM_ARRAYSIZE(roomTypes));
    ImGui::SliderInt("Number of Nights", &gDays, 1, 30);
    ImGui::SliderInt("Guests", &gGuests, 1, 5);
    ImGui::SliderInt("Rooms", &gRooms, 1, 3);
    ImGui::SliderInt("Beds", &gBeds, 1, 5);

    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::Spacing();

    float totalBill = roomPrices[roomType] * gDays * gRooms + gBeds * 50.0f;

    ImGui::BeginChild("BillPreview", ImVec2(0, 100), true);
    ImGui::Columns(2, "billCol", false);

    ImGui::Text("ESTIMATED TOTAL:");
    ImGui::SetWindowFontScale(2.0f);
    ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "$ %.2f", totalBill);
    ImGui::SetWindowFontScale(1.0f);

    ImGui::NextColumn();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);
    if (ImGui::Button("CONFIRM RESERVATION", ImVec2(-1, 50))) {
        if (strlen(gName) == 0) {

        }
        else {
            Reservation newRes;
            newRes.id = (int)reservations.size() + 1;
            strncpy_s(newRes.customerName, gName, sizeof(newRes.customerName) - 1);
            newRes.tableNumber = (int)reservations.size() + 101;
            newRes.guests = gDays;
            addReservation(newRes);

            gName[0] = '\0';
            gPhone[0] = '\0';
            gEmail[0] = '\0';
            gDays = 1;
            gGuests = 1;
            gRooms = 1;
            gBeds = 1;
        }
    }

    ImGui::EndChild();

    ImGui::EndChild();
    ImGui::PopStyleColor();

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
            AuthSession& session = getAuthSession();
            if(session.currentUser.role == UserRole::Admin) {
                renderUI();         // existing reservation UI
            } else {
                renderGuestUI();    // guest-specific UI
            }
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