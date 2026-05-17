#define _CRT_SECURE_NO_WARNINGS
#include "presentation.h"
#include "auth.h"
#include "data.h"
#include "imgui.h"
#include "logic.h"
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

// ─── Theme ───────────────────────────────────────────────────────────────────

void setupModernTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.ChildRounding = 10.0f;
    style.FrameRounding = 8.0f;
    style.PopupRounding = 8.0f;
    style.GrabRounding = 8.0f;
    style.ItemSpacing = ImVec2(12, 12);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.04f, 0.05f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.15f, 0.25f, 0.45f, 0.60f);
    colors[ImGuiCol_Separator] = ImVec4(0.15f, 0.25f, 0.45f, 0.60f);
    colors[ImGuiCol_Button] = ImVec4(0.12f, 0.18f, 0.30f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.35f, 0.60f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.08f, 0.12f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.14f, 0.22f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.35f, 0.60f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.15f, 0.28f, 0.50f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.07f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.30f, 0.50f, 1.00f);
}

// ─── Date helpers ─────────────────────────────────────────────────────────────

static void formatDateFromTime(time_t t, char* buf, size_t size) {
    struct tm ts;
    localtime_s(&ts, &t);
    strftime(buf, size, "%d/%m/%Y", &ts);
}

static void formatTimeFromTime(time_t t, char* buf, size_t size) {
    struct tm ts;
    localtime_s(&ts, &t);
    strftime(buf, size, "%H:%M", &ts);
}

/*
 * parseDateDDMMYYYY
 *
 * Converts a "DD/MM/YYYY" string into a time_t at midnight.
 * Returns 0 on parse failure.
 */
static time_t parseDateDDMMYYYY(const char* dateStr) {
    int d = 0, m = 0, y = 0;
    if (sscanf_s(dateStr, "%d/%d/%d", &d, &m, &y) != 3) return 0;
    struct tm t = {};
    t.tm_mday = d;
    t.tm_mon = m - 1;
    t.tm_year = y - 1900;
    t.tm_hour = 0; t.tm_min = 0; t.tm_sec = 0;
    t.tm_isdst = -1;
    return mktime(&t);
}

/*
 * midnightOf
 *
 * Returns a time_t at midnight (00:00:00) of the day that [t] falls on.
 */
static time_t midnightOf(time_t t) {
    struct tm ts;
    localtime_s(&ts, &t);
    ts.tm_hour = 0; ts.tm_min = 0; ts.tm_sec = 0;
    ts.tm_isdst = -1;
    return mktime(&ts);
}

// ─── Schedule tab state ───────────────────────────────────────────────────────

/* Number of days to display in the schedule grid (7, 14, or 30) */
static int  s_schedDays = 14;

/*
 * s_schedStartOffset – how many days from today the visible window starts.
 * Negative values scroll backward into the past.
 */
static int  s_schedStartOffset = -3;

/*
 * Block colour palette – one colour per reservation slot.
 * Each entry is (background R,G,B) with a lighter text tint.
 */
struct BlockColor {
    ImVec4 bg;
    ImVec4 border;
    ImVec4 text;
};

static const BlockColor BLOCK_COLORS[] = {
    { ImVec4(0.10f,0.23f,0.42f,1), ImVec4(0.23f,0.42f,0.67f,1), ImVec4(0.55f,0.80f,1.00f,1) },
    { ImVec4(0.10f,0.29f,0.17f,1), ImVec4(0.23f,0.55f,0.30f,1), ImVec4(0.55f,0.87f,0.55f,1) },
    { ImVec4(0.29f,0.17f,0.10f,1), ImVec4(0.55f,0.35f,0.23f,1), ImVec4(0.87f,0.67f,0.55f,1) },
    { ImVec4(0.23f,0.10f,0.29f,1), ImVec4(0.48f,0.29f,0.55f,1), ImVec4(0.80f,0.55f,1.00f,1) },
    { ImVec4(0.29f,0.10f,0.17f,1), ImVec4(0.55f,0.23f,0.36f,1), ImVec4(1.00f,0.55f,0.67f,1) },
    { ImVec4(0.10f,0.29f,0.29f,1), ImVec4(0.23f,0.55f,0.55f,1), ImVec4(0.55f,0.87f,0.87f,1) },
    { ImVec4(0.23f,0.23f,0.10f,1), ImVec4(0.48f,0.48f,0.23f,1), ImVec4(0.87f,0.87f,0.55f,1) },
    { ImVec4(0.17f,0.10f,0.29f,1), ImVec4(0.36f,0.23f,0.55f,1), ImVec4(0.67f,0.55f,1.00f,1) },
};
static const int NUM_BLOCK_COLORS = 8;

// ─── Schedule renderer ────────────────────────────────────────────────────────

/*
 * Drag-to-pan state for the calendar grid.
 *
 * s_dragActive     – true while the left mouse button is held on the grid.
 * s_dragStartX     – screen X position where the drag began.
 * s_dragScrollBase – value of ImGui's horizontal scroll at drag start.
 *
 * Each frame we compute how many pixels the mouse has moved since drag start
 * and set the scroll to (s_dragScrollBase - delta), giving a natural "grab
 * and pull" feel.  We do NOT update s_schedStartOffset during a drag so that
 * the toolbar buttons still reflect the correct logical window position.
 */
static bool  s_dragActive = false;
static float s_dragStartX = 0.0f;
static float s_dragScrollBase = 0.0f;
static float s_dragStartY = 0.0f;
static float s_dragScrollBaseY = 0.0f;

/* Zoom state – pixels per day column */
static float s_schedZoom = 1.0f;   /* 1.0 = 100 %, range 0.5 – 3.0 */

/*
 * renderScheduleTab
 *
 * Draws a calendar-style occupancy grid:
 *   - Rows    = rooms (fixed left column, sticky)
 *   - Columns = days  (scrollable horizontally)
 *   - Blocks  = reservations spanning their check-in → check-out range
 *
 * Drag to pan: click and hold anywhere on the grid and drag left/right.
 *
 * Only the data layer (getReservations) is touched through the logic layer
 * wrapper; the presentation layer never accesses raw storage directly.
 */
static void renderScheduleTab() {
    auto& reservations = getReservations();

    /* ── Toolbar ── */
    ImGui::Spacing();

    /* Navigation buttons */
    if (ImGui::Button("< Prev week"))  s_schedStartOffset -= 7;
    ImGui::SameLine();
    if (ImGui::Button("Today"))        s_schedStartOffset = -3;
    ImGui::SameLine();
    if (ImGui::Button("Next week >"))  s_schedStartOffset += 7;
    ImGui::SameLine();

    /* Days-visible selector */
    ImGui::Text("  Show:");
    ImGui::SameLine();
    if (ImGui::RadioButton("7d", s_schedDays == 7))  s_schedDays = 7;
    ImGui::SameLine();
    if (ImGui::RadioButton("14d", s_schedDays == 14)) s_schedDays = 14;
    ImGui::SameLine();
    if (ImGui::RadioButton("30d", s_schedDays == 30)) s_schedDays = 30;

    ImGui::SameLine();
    ImGui::Text("  Zoom:");
    ImGui::SameLine();
    if (ImGui::Button(" - ##zm")) { s_schedZoom -= 0.25f; if (s_schedZoom < 0.5f) s_schedZoom = 0.5f; }
    ImGui::SameLine(0, 4);
    ImGui::SetNextItemWidth(100.0f);
    ImGui::SliderFloat("##zoom", &s_schedZoom, 0.5f, 3.0f, "%.2fx");
    ImGui::SameLine(0, 4);
    if (ImGui::Button(" + ##zm")) { s_schedZoom += 0.25f; if (s_schedZoom > 3.0f) s_schedZoom = 3.0f; }
    ImGui::SameLine(0, 8);
    if (ImGui::Button("Reset##zm")) s_schedZoom = 1.0f;

    /* Ctrl+scroll to zoom */
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) &&
        ImGui::GetIO().KeyCtrl && ImGui::GetIO().MouseWheel != 0.0f) {
        s_schedZoom += ImGui::GetIO().MouseWheel * 0.15f;
        if (s_schedZoom < 0.5f) s_schedZoom = 0.5f;
        if (s_schedZoom > 3.0f) s_schedZoom = 3.0f;
    }

    ImGui::Spacing();
    ImGui::Separator();

    /* ── Layout constants ── */
    const float ROOM_LABEL_W = 110.0f; /* width of the frozen room-name column */
    const float DAY_CELL_W = 90.0f * s_schedZoom;  /* width of each day column (zoomed) */
    const float ROW_H = 56.0f * (s_schedZoom > 1.0f ? s_schedZoom : 1.0f);  /* height of each room row (zoomed vertically) */
    const float DAY_HDR_H = 24.0f;  /* height of the day-name header row    */

    /* Compute start time (midnight of the offset day) */
    time_t now = time(nullptr);
    time_t todayMid = midnightOf(now);
    time_t startMid = todayMid + (time_t)s_schedStartOffset * 86400;

    /* Total inner width of the scrollable area */
    float innerW = ROOM_LABEL_W + (float)s_schedDays * DAY_CELL_W;

    /* Collect unique room numbers from reservations for the row list */
    static const int KNOWN_ROOMS[] = {
        101, 102, 103, 201, 202, 203, 204, 205,
        301, 302, 401, 402, 501
    };
    static const int NUM_KNOWN_ROOMS = 13;

    /* ── Outer scrollable child ── */
    ImVec2 availSize = ImGui::GetContentRegionAvail();

    /*
     * NoScrollWithMouse prevents ImGui from consuming the scroll wheel on this
     * child so that drag input is the primary way to pan.  The scrollbar is
     * still visible so users can also drag it directly.
     */
    ImGui::BeginChild("##schedOuter", availSize, false,
        ImGuiWindowFlags_HorizontalScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 canvasOrigin = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();

    /* ── Drag-to-pan logic ──
     *
     * We use an invisible full-area button so ImGui tracks hover/active state
     * for us correctly even when the mouse moves fast.  The button is drawn
     * BEFORE any other content so it sits at the bottom of the z-order and
     * does not block the reservation InvisibleButtons used for tooltips.
     * Dragging works both horizontally AND vertically.
     */
    {
        /* Save cursor, draw the hit-test rect, then restore */
        float totalH = DAY_HDR_H + (float)NUM_KNOWN_ROOMS * ROW_H;
        ImVec2 savedCursor = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(0, 0));
        ImGui::InvisibleButton("##dragArea", ImVec2(innerW, totalH));
        ImGui::SetCursorPos(savedCursor);

        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 1.0f)) {
            if (!s_dragActive) {
                /* Drag just started – record baseline */
                s_dragActive = true;
                s_dragStartX = ImGui::GetMousePos().x;
                s_dragScrollBase = ImGui::GetScrollX();
                s_dragStartY = ImGui::GetMousePos().y;
                s_dragScrollBaseY = ImGui::GetScrollY();
            }
            float deltaX = ImGui::GetMousePos().x - s_dragStartX;
            float deltaY = ImGui::GetMousePos().y - s_dragStartY;
            ImGui::SetScrollX(s_dragScrollBase - deltaX);
            ImGui::SetScrollY(s_dragScrollBaseY - deltaY);

            /* Change cursor to a grabbing hand while dragging */
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        }
        else {
            s_dragActive = false;
        }
    }

    /* ── Draw day-header row (sticky: unaffected by vertical scroll) ── */
    float headerY = canvasOrigin.y - scrollY + DAY_HDR_H;

    /* Corner cell background */
    dl->AddRectFilled(
        ImVec2(canvasOrigin.x - scrollX, canvasOrigin.y - scrollY),
        ImVec2(canvasOrigin.x - scrollX + ROOM_LABEL_W, headerY),
        IM_COL32(18, 18, 30, 255));

    /* Day columns header */
    for (int di = 0; di < s_schedDays; di++) {
        time_t dayT = startMid + (time_t)di * 86400;
        bool   isToday = (dayT == todayMid);

        float cellX = canvasOrigin.x - scrollX + ROOM_LABEL_W + (float)di * DAY_CELL_W;
        float cellY = canvasOrigin.y - scrollY;

        /* Cell background */
        ImU32 bgColor = isToday
            ? IM_COL32(40, 38, 10, 255)
            : IM_COL32(22, 22, 40, 255);
        dl->AddRectFilled(ImVec2(cellX, cellY),
            ImVec2(cellX + DAY_CELL_W, cellY + DAY_HDR_H), bgColor);
        dl->AddLine(ImVec2(cellX + DAY_CELL_W, cellY),
            ImVec2(cellX + DAY_CELL_W, cellY + DAY_HDR_H),
            IM_COL32(42, 42, 60, 255));

        /* Day label: "Fri 17/5" */
        struct tm dtm;
        localtime_s(&dtm, &dayT);
        char dayLabel[16];
        strftime(dayLabel, sizeof(dayLabel), "%a %d/%m", &dtm);

        ImU32 textColor = isToday
            ? IM_COL32(255, 220, 50, 255)
            : IM_COL32(140, 140, 160, 255);
        dl->AddText(ImVec2(cellX + 4, cellY + 5), textColor, dayLabel);
    }

    /* ── Draw room rows ── */
    for (int ri = 0; ri < NUM_KNOWN_ROOMS; ri++) {
        int   roomNum = KNOWN_ROOMS[ri];
        float rowY = canvasOrigin.y + DAY_HDR_H + (float)ri * ROW_H;

        /* Alternating row backgrounds */
        ImU32 rowBg = (ri % 2 == 0)
            ? IM_COL32(13, 13, 18, 255)
            : IM_COL32(16, 16, 24, 255);
        dl->AddRectFilled(
            ImVec2(canvasOrigin.x - scrollX, rowY),
            ImVec2(canvasOrigin.x - scrollX + innerW, rowY + ROW_H),
            rowBg);

        /* Day separator lines */
        for (int di = 0; di <= s_schedDays; di++) {
            float lx = canvasOrigin.x - scrollX + ROOM_LABEL_W + (float)di * DAY_CELL_W;
            bool  isToday = (startMid + (time_t)di * 86400 == todayMid);
            ImU32 lineCol = isToday
                ? IM_COL32(80, 78, 20, 255)
                : IM_COL32(28, 28, 42, 255);
            dl->AddLine(ImVec2(lx, rowY),
                ImVec2(lx, rowY + ROW_H), lineCol);
        }

        /* Bottom border */
        dl->AddLine(ImVec2(canvasOrigin.x - scrollX, rowY + ROW_H),
            ImVec2(canvasOrigin.x - scrollX + innerW, rowY + ROW_H),
            IM_COL32(28, 28, 42, 255));

        /* ── Frozen room label (sticky left) ── */
        float labelX = canvasOrigin.x - scrollX;
        dl->AddRectFilled(
            ImVec2(labelX, rowY),
            ImVec2(labelX + ROOM_LABEL_W, rowY + ROW_H),
            IM_COL32(18, 18, 30, 255));
        dl->AddLine(ImVec2(labelX + ROOM_LABEL_W, rowY),
            ImVec2(labelX + ROOM_LABEL_W, rowY + ROW_H),
            IM_COL32(42, 42, 70, 255));

        /* Room number + type hint */
        char roomLabel[16];
        snprintf(roomLabel, sizeof(roomLabel), "Room %d", roomNum);
        const char* typeHint =
            roomNum >= 500 ? "Presidential" :
            roomNum >= 400 ? "Suite" :
            roomNum >= 300 ? "Suite" :
            roomNum >= 200 ? "Deluxe" : "Standard";

        dl->AddText(ImVec2(labelX + 8, rowY + 8),
            IM_COL32(136, 170, 221, 255), roomLabel);
        dl->AddText(ImVec2(labelX + 8, rowY + 26),
            IM_COL32(80, 80, 100, 255), typeHint);

        /* ── Reservation blocks for this room ── */
        for (int resIdx = 0; resIdx < (int)reservations.size(); resIdx++) {
            const Reservation& r = reservations[resIdx];
            if (r.tableNumber != roomNum) continue;

            time_t ciT = parseDateDDMMYYYY(r.checkIn);
            time_t coT = parseDateDDMMYYYY(r.checkOut);
            if (ciT == 0 || coT == 0) continue;

            time_t visEnd = startMid + (time_t)s_schedDays * 86400;
            if (coT <= startMid || ciT >= visEnd) continue; /* out of view */

            /* Clamp to visible window */
            float startFrac = (float)(ciT - startMid) / 86400.0f;
            float endFrac = (float)(coT - startMid) / 86400.0f;
            startFrac = (startFrac < 0) ? 0.0f : startFrac;
            endFrac = (endFrac > (float)s_schedDays) ? (float)s_schedDays : endFrac;

            float bx1 = canvasOrigin.x - scrollX + ROOM_LABEL_W + startFrac * DAY_CELL_W + 2;
            float bx2 = canvasOrigin.x - scrollX + ROOM_LABEL_W + endFrac * DAY_CELL_W - 2;
            float by1 = rowY + 5;
            float by2 = rowY + ROW_H - 5;

            if (bx2 - bx1 < 4) continue; /* too narrow to draw */

            /* Colour from palette */
            const BlockColor& c = BLOCK_COLORS[resIdx % NUM_BLOCK_COLORS];
            ImU32 bgCol = ImGui::ColorConvertFloat4ToU32(c.bg);
            ImU32 borCol = ImGui::ColorConvertFloat4ToU32(c.border);
            ImU32 txtCol = ImGui::ColorConvertFloat4ToU32(c.text);

            dl->AddRectFilled(ImVec2(bx1, by1), ImVec2(bx2, by2), bgCol, 4.0f);
            dl->AddRect(ImVec2(bx1, by1), ImVec2(bx2, by2), borCol, 4.0f);

            /* Guest name (clipped to block width) */
            float textW = bx2 - bx1 - 8;
            if (textW > 20) {
                /* Push a clipping rectangle so text doesn't bleed out */
                dl->PushClipRect(ImVec2(bx1 + 4, by1),
                    ImVec2(bx1 + 4 + textW, by2), true);
                dl->AddText(ImVec2(bx1 + 5, by1 + 5), txtCol, r.customerName);

                /* Check-in / check-out time line */
                char timeLabel[24];
                snprintf(timeLabel, sizeof(timeLabel), "%s -> 11:00",
                    r.checkInTime[0] ? r.checkInTime : "14:00");
                dl->AddText(ImVec2(bx1 + 5, by1 + 21),
                    IM_COL32(180, 180, 180, 160), timeLabel);
                dl->PopClipRect();
            }

            /* ── Invisible hover button for tooltip ── */
            ImGui::SetCursorScreenPos(ImVec2(bx1, by1));
            ImGui::InvisibleButton(
                (std::string("##rb") + std::to_string(resIdx) + "_" + std::to_string(ri)).c_str(),
                ImVec2(bx2 - bx1, by2 - by1));

            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::TextColored(ImVec4(1, 1, 1, 1), "%s", r.customerName);
                ImGui::Separator();
                ImGui::Text("Room:       %d (%s)", r.tableNumber, typeHint);
                ImGui::Text("Check-in:   %s at %s", r.checkIn,
                    r.checkInTime[0] ? r.checkInTime : "14:00");
                ImGui::Text("Check-out:  %s at 11:00", r.checkOut);
                ImGui::Text("Nights:     %d", r.guests);
                ImGui::Text("Type:       %s", r.roomType);
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f),
                    "Bill:       $ %.2f", r.totalBill);
                ImGui::EndTooltip();
            }
        }
    }

    /* Set the dummy size so ImGui knows the scrollable area */
    ImGui::SetCursorScreenPos(ImVec2(canvasOrigin.x,
        canvasOrigin.y + DAY_HDR_H +
        (float)NUM_KNOWN_ROOMS * ROW_H));
    ImGui::Dummy(ImVec2(innerW, 1));

    ImGui::EndChild();
}

// ─── Spinbox helper ───────────────────────────────────────────────────────────

/*
 * SpinInt
 *
 * A compact integer input with visible +/- buttons that do not overlap
 * the number.  [label] must be unique (used as ImGui ID).
 * Returns true if the value changed this frame.
 */
static bool SpinInt(const char* label, int* value, int minVal, int maxVal, int step = 1) {
    bool changed = false;

    ImGui::PushID(label);

    /* − button */
    if (ImGui::Button("-", ImVec2(26, 26))) {
        *value -= step;
        if (*value < minVal) *value = minVal;
        changed = true;
    }

    ImGui::SameLine(0, 4);

    /* Number field – fixed width so buttons never touch the digits */
    ImGui::SetNextItemWidth(52);
    if (ImGui::InputInt("##v", value, 0, 0)) {
        if (*value < minVal) *value = minVal;
        if (*value > maxVal) *value = maxVal;
        changed = true;
    }

    ImGui::SameLine(0, 4);

    /* + button */
    if (ImGui::Button("+", ImVec2(26, 26))) {
        *value += step;
        if (*value > maxVal) *value = maxVal;
        changed = true;
    }

    ImGui::PopID();
    return changed;
}

// ─── Admin UI ─────────────────────────────────────────────────────────────────

void renderUI() {
    setupModernTheme();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGuiWindowFlags wf = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    static int  searchResultIdx = -2;
    static int  searchId = 0;
    static int  deleteTargetId = -1;
    static char deleteTargetName[64] = "";
    auto& reservations = getReservations();

    ImGui::Begin("Hotel Management System", nullptr, wf);

    /* ── Header ── */
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 0.84f, 0.00f, 1.00f));
    ImGui::SetWindowFontScale(1.4f);
    ImGui::Text("HOTEL NullTerminators - EXECUTIVE MANAGEMENT");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    AuthSession& session = getAuthSession();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1.0f),
        "System Version 2.1 | Logged in as: %s",
        session.currentUser.role == UserRole::Admin ? "Admin" : "Guest");

    /* Log Out – flush right, vertically centred on the header line */
    float logoffWidth = 100.0f;
    float logoffHeight = 28.0f;
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - logoffWidth);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.05f, 0.05f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.10f, 0.10f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.03f, 0.03f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 0.60f, 0.60f, 1.00f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 14.0f);
    if (ImGui::Button("[ Log Out ]", ImVec2(logoffWidth, logoffHeight))) logoutAuth();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);

    ImGui::Separator();
    ImGui::Spacing();

    /* ── Stat cards ── */
    ImGui::Columns(3, "stats", false);

    ImGui::BeginChild("Stat1", ImVec2(0, 80), true);
    ImGui::Text("TOTAL RESERVATIONS");
    ImGui::SetWindowFontScale(1.9f);
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%zu", reservations.size());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::EndChild();

    ImGui::NextColumn();
    ImGui::BeginChild("Stat2", ImVec2(0, 80), true);
    ImGui::Text("GUESTS IN HOUSE");
    ImGui::SetWindowFontScale(1.9f);
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%d",
        calculateTotalGuestsRecursive(reservations, 0));
    ImGui::SetWindowFontScale(1.0f);
    ImGui::EndChild();

    ImGui::NextColumn();
    ImGui::BeginChild("Stat3", ImVec2(0, 80), true);
    ImGui::Text("AVAILABLE ROOMS");
    ImGui::SetWindowFontScale(1.9f);
    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
        "%d / 500", 500 - (int)reservations.size());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::EndChild();

    ImGui::Columns(1);
    ImGui::Spacing();

    /* ── Tab bar ── */
    if (ImGui::BeginTabBar("AdminTabs")) {

        // ════════════════════════════════════════════════════════════════════
        // TAB 1 – CURRENT OCCUPANCY
        // ════════════════════════════════════════════════════════════════════
        if (ImGui::BeginTabItem("Current Occupancy")) {

            ImGui::Columns(2, "mainLayout", true);
            ImGui::SetColumnWidth(0, 300);

            /* Left: quick actions */
            ImGui::Text("QUICK ACTIONS");
            if (ImGui::Button("Sort Guest List", ImVec2(-1, 40))) sortByName();

            ImGui::Spacing();
            ImGui::Text("Search by ID:");
            ImGui::SetNextItemWidth(-1);
            ImGui::InputInt("##searchId", &searchId, 0, 0);
            if (searchId < 0) searchId = 0;

            ImGui::Spacing();
            if (ImGui::Button("FIND", ImVec2(-1, 45)))
                searchResultIdx = findReservationById(searchId);
            if (ImGui::Button("RESET", ImVec2(-1, 35))) {
                searchResultIdx = -2; searchId = 0;
            }

            ImGui::NextColumn();

            /* Right: table */
            ImGui::Text("CURRENT OCCUPANCY");
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.25f, 0.45f, 0.30f));

            if (ImGui::BeginTable("GuestTable", 10,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
                ImVec2(0, -10))) {

                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 35.0f);
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Phone", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn("Room", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                ImGui::TableSetupColumn("Nights", ImGuiTableColumnFlags_WidthFixed, 55.0f);
                ImGui::TableSetupColumn("Check-In", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                ImGui::TableSetupColumn("CI Time", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableSetupColumn("Check-Out", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableHeadersRow();

                for (int i = 0; i < (int)reservations.size(); i++) {
                    ImGui::TableNextRow(0, 32.0f);
                    if (i == searchResultIdx)
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
                            ImColor(50, 150, 50, 180));

                    ImGui::TableSetColumnIndex(0); ImGui::Text("%d", reservations[i].id);
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%s", reservations[i].customerName);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "%s",
                        reservations[i].phone[0] ? reservations[i].phone : "-");
                    ImGui::TableSetColumnIndex(3); ImGui::Text("%d", reservations[i].tableNumber);
                    ImGui::TableSetColumnIndex(4); ImGui::Text("%d", reservations[i].guests);
                    ImGui::TableSetColumnIndex(5); ImGui::Text("%s", reservations[i].checkIn);
                    ImGui::TableSetColumnIndex(6);
                    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%s",
                        reservations[i].checkInTime[0] ? reservations[i].checkInTime : "14:00");
                    ImGui::TableSetColumnIndex(7); ImGui::Text("%s", reservations[i].checkOut);
                    ImGui::TableSetColumnIndex(8); ImGui::Text("%s", reservations[i].roomType);

                    ImGui::TableSetColumnIndex(9);
                    ImGui::PushID(i);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.08f, 0.08f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.60f, 0.12f, 0.12f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.05f, 0.05f, 1.0f));
                    if (ImGui::SmallButton("Delete")) {
                        deleteTargetId = reservations[i].id;
                        strncpy_s(deleteTargetName, reservations[i].customerName,
                            sizeof(deleteTargetName) - 1);
                    }
                    ImGui::PopStyleColor(3);
                    ImGui::PopID();
                }

                ImGui::EndTable();
            }

            /* Open the popup OUTSIDE the table so the ID stack is clean */
            if (deleteTargetId != -1 && strcmp(deleteTargetName, "") != 0)
                ImGui::OpenPopup("Confirm Delete");

            /* Delete confirmation modal */
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(340, 0), ImGuiCond_Always);
            if (ImGui::BeginPopupModal("Confirm Delete", nullptr,
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {

                ImGui::Spacing();
                ImGui::SetCursorPosX(
                    (ImGui::GetContentRegionAvail().x -
                        ImGui::CalcTextSize("Delete Reservation?").x) * 0.5f);
                ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.0f), "Delete Reservation?");
                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
                ImGui::Text("Guest:"); ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s", deleteTargetName);
                ImGui::Spacing();
                ImGui::TextDisabled("This action cannot be undone.");
                ImGui::Spacing();

                float btnW = (ImGui::GetContentRegionAvail().x - 8.0f) * 0.5f;
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.08f, 0.08f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.60f, 0.12f, 0.12f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.05f, 0.05f, 1.0f));
                if (ImGui::Button("Delete", ImVec2(btnW, 36))) {
                    deleteReservation(deleteTargetId);
                    if (searchResultIdx >= 0) searchResultIdx = -2;
                    deleteTargetId = -1;
                    deleteTargetName[0] = '\0';
                    ImGui::CloseCurrentPopup();
                }
                ImGui::PopStyleColor(3);
                ImGui::SameLine(0, 8);
                if (ImGui::Button("Cancel", ImVec2(btnW, 36))) {
                    deleteTargetId = -1;
                    deleteTargetName[0] = '\0';
                    ImGui::CloseCurrentPopup();
                }
                ImGui::Spacing();
                ImGui::EndPopup();
            }

            ImGui::PopStyleColor();
            ImGui::Columns(1);
            ImGui::EndTabItem();
        }

        // ════════════════════════════════════════════════════════════════════
        // TAB 2 – SCHEDULE  (calendar grid)
        // ════════════════════════════════════════════════════════════════════
        if (ImGui::BeginTabItem("Schedule")) {
            renderScheduleTab();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

// ─── Guest UI ─────────────────────────────────────────────────────────────────

void renderGuestUI() {
    setupModernTheme();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGuiWindowFlags wf = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    static char gName[64] = "";
    static char gPhone[32] = "";
    static char gEmail[64] = "";

    /* Night / guest / room / bed counts */
    static int gDays = 1;
    static int gGuests = 1;
    static int gRooms = 1;
    static int gBeds = 1;
    static int gCheckInHour = 14;
    static int gCheckInMin = 0;
    static int roomType = 0;
    /* Days from today the guest wants to check in (0 = today) */
    static int gCheckInOffset = 0;

    const char* roomTypes[] = { "Standard Room", "Deluxe Room", "Presidential Suite" };
    float       roomPrices[] = { 100.0f, 200.0f, 500.0f };

    time_t now = time(nullptr);
    time_t todayMidnight = midnightOf(now);
    time_t checkInDayT = todayMidnight + (time_t)gCheckInOffset * 86400;
    time_t checkOutT = checkInDayT + (static_cast<long long>(gDays) * 86400);

    char startDate[16], endDate[16], nowTime[8];
    formatDateFromTime(checkInDayT, startDate, sizeof(startDate));
    formatDateFromTime(checkOutT, endDate, sizeof(endDate));
    formatTimeFromTime(now, nowTime, sizeof(nowTime));

    char chosenCheckInTime[8];
    snprintf(chosenCheckInTime, sizeof(chosenCheckInTime),
        "%02d:%02d", gCheckInHour, gCheckInMin);

    auto& reservations = getReservations();
    AuthSession& session = getAuthSession();

    ImGui::Begin("Guest Portal", nullptr, wf);

    /* ── Header row ── */
    ImGui::Spacing();

    /* Welcome greeting – rendered directly, no child box */
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 0.84f, 0.00f, 1.00f));
    ImGui::SetWindowFontScale(1.4f);
    ImGui::Text("HOTEL NullTerminators - GUEST PORTAL");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    /* "Welcome, <FirstName>" – inline below the title, no box */
    ImGui::Text("Welcome,");
    ImGui::SameLine(0, 6);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.40f, 0.85f, 1.00f, 1.00f));
    ImGui::SetWindowFontScale(1.25f);
    ImGui::Text("%s", session.currentUser.firstName.c_str());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    ImGui::SameLine();

    /* Log Out – styled pill button, flush right */
    float logoffWidth = 100.0f;
    float logoffHeight = 28.0f;
    ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - logoffWidth);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.05f, 0.05f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.10f, 0.10f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.03f, 0.03f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 0.60f, 0.60f, 1.00f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 14.0f);
    if (ImGui::Button("[ Log Out ]##Guest", ImVec2(logoffWidth, logoffHeight))) logoutAuth();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);

    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Status: Online");

    ImGui::Separator();
    ImGui::Spacing();

    /* Stat cards – only 2 now (welcome moved to header) */
    ImGui::Columns(2, "gStats", false);

    ImGui::BeginChild("gS2", ImVec2(0, 76), true);
    ImGui::Text("AVAILABLE SLOTS");
    ImGui::SetWindowFontScale(1.8f);
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f),
        "%d", 500 - (int)reservations.size());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::EndChild();

    ImGui::NextColumn();

    /* Date/time card – always shows today's date, no scrollbar */
    {
        char todayStr[16], todayTime[8];
        formatDateFromTime(todayMidnight, todayStr, sizeof(todayStr));
        formatTimeFromTime(now, todayTime, sizeof(todayTime));
        ImGui::BeginChild("gS3", ImVec2(0, 76), true,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::TextDisabled("TODAY");
        ImGui::SetWindowFontScale(1.25f);
        ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), "%s", todayStr);
        ImGui::SetWindowFontScale(1.0f);
        ImGui::TextDisabled("Local time: %s", todayTime);
        ImGui::EndChild();
    }

    ImGui::Columns(1);
    ImGui::Spacing();

    /* Existing reservation card */
    bool        hasExisting = false;
    Reservation existingRes{};
    std::string loggedInName = session.currentUser.firstName + " " + session.currentUser.lastName;
    std::string loggedInEmail = session.currentUser.email;

    for (const auto& res : reservations) {
        std::string resName(res.customerName);
        if (resName.find(loggedInName) != std::string::npos ||
            resName.find(loggedInEmail) != std::string::npos) {
            hasExisting = true; existingRes = res; break;
        }
    }

    if (hasExisting) {
        ImGui::Text("YOUR CURRENT RESERVATION");
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.20f, 0.15f, 1.00f));
        ImGui::BeginChild("ActiveRes", ImVec2(0, 90), true);
        ImGui::Columns(5, "rInfo", false);

        ImGui::TextDisabled("ROOM");
        ImGui::Text("Room %d", existingRes.tableNumber);
        ImGui::NextColumn();

        ImGui::TextDisabled("CHECK-IN");
        ImGui::Text("%s", existingRes.checkIn);
        ImGui::NextColumn();

        ImGui::TextDisabled("CHECK-OUT");
        ImGui::Text("%s", existingRes.checkOut);
        ImGui::NextColumn();

        ImGui::TextDisabled("TOTAL PAID");
        ImGui::TextColored(ImVec4(1.00f, 0.84f, 0.00f, 1.00f), "$ %.2f", existingRes.totalBill);
        ImGui::NextColumn();

        ImGui::TextDisabled("STATUS");
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Confirmed");

        ImGui::Columns(1);
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    /* Booking form */
    ImGui::Text("MAKE A NEW RESERVATION");
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.12f, 1.00f));
    ImGui::BeginChild("BookingForm", ImVec2(0, 0), true);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Columns(2, "formSplit", false);

    /* Pre-fill name / email from session on first open */
    if (gName[0] == '\0') {
        std::string full = session.currentUser.firstName + " " + session.currentUser.lastName;
        strncpy_s(gName, full.c_str(), sizeof(gName) - 1);
    }
    if (gEmail[0] == '\0')
        strncpy_s(gEmail, session.currentUser.email.c_str(), sizeof(gEmail) - 1);

    ImGui::InputTextWithHint("##name", "Full Name", gName, 64);
    ImGui::InputTextWithHint("##phone", "Phone Number", gPhone, 32);
    ImGui::InputTextWithHint("##email", "Email", gEmail, 64);
    ImGui::Spacing();

    /* ── Check-in date – compact Prev / date / Next ── */
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Check-in date:");
    {
        bool changed = false;

        char ciLabel[40];
        if (gCheckInOffset == 0)      snprintf(ciLabel, sizeof(ciLabel), "Today  %s", startDate);
        else if (gCheckInOffset == 1) snprintf(ciLabel, sizeof(ciLabel), "Tomorrow  %s", startDate);
        else                          snprintf(ciLabel, sizeof(ciLabel), "+%d days  %s", gCheckInOffset, startDate);

        const float btnW = 58.0f;
        const float btnH = 24.0f;
        const float spacing = 8.0f;

        if (ImGui::Button("< Prev##d", ImVec2(btnW, btnH))) {
            if (gCheckInOffset > 0) { gCheckInOffset--; changed = true; }
        }
        ImGui::SameLine(0, spacing);

        /* AlignTextToFramePadding centres the text vertically against the button height */
        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), "%s", ciLabel);

        ImGui::SameLine(0, spacing);
        if (ImGui::Button("Next >##d", ImVec2(btnW, btnH))) {
            if (gCheckInOffset < 365) { gCheckInOffset++; changed = true; }
        }

        if (changed) {
            checkInDayT = todayMidnight + (time_t)gCheckInOffset * 86400;
            formatDateFromTime(checkInDayT, startDate, sizeof(startDate));
            checkOutT = checkInDayT + (static_cast<long long>(gDays) * 86400);
            formatDateFromTime(checkOutT, endDate, sizeof(endDate));
        }
    }

    ImGui::Spacing();

    /* ── Check-in time – all items on one baseline via AlignTextToFramePadding ── */
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Check-in time:");
    ImGui::SameLine(0, 8);
    SpinInt("##ciH", &gCheckInHour, 0, 23, 1);
    ImGui::SameLine(0, 6);
    ImGui::AlignTextToFramePadding();
    ImGui::Text(":");
    ImGui::SameLine(0, 6);
    {
        int ciMinIdx = gCheckInMin / 15;
        if (SpinInt("##ciM", &ciMinIdx, 0, 3, 1))
            gCheckInMin = ciMinIdx * 15;
    }
    ImGui::SameLine(0, 10);
    ImGui::AlignTextToFramePadding();
    ImGui::TextDisabled("%02d:%02d", gCheckInHour, gCheckInMin);

    /* ── Check-out ── */
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
        "Check-out: %s at 11:00  (%d night%s)",
        endDate, gDays, gDays == 1 ? "" : "s");

    ImGui::NextColumn();

    /* ── Stay details (all sliders) ── */
    ImGui::Text("Stay Details");
    ImGui::Combo("Room Type", &roomType, roomTypes, IM_ARRAYSIZE(roomTypes));
    ImGui::Spacing();

    ImGui::Text("Number of Nights:  %d", gDays);
    ImGui::SetNextItemWidth(-1);
    if (ImGui::SliderInt("##nights", &gDays, 1, 90)) {
        checkOutT = checkInDayT + (static_cast<long long>(gDays) * 86400);
        formatDateFromTime(checkOutT, endDate, sizeof(endDate));
    }

    ImGui::Spacing();
    ImGui::Text("Guests:  %d", gGuests);
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##guests", &gGuests, 1, 10);

    ImGui::Spacing();
    ImGui::Text("Rooms:  %d", gRooms);
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##rooms", &gRooms, 1, 5);

    ImGui::Spacing();
    ImGui::Text("Beds:  %d", gBeds);
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##beds", &gBeds, 1, 10);

    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::Spacing();

    float totalBill = roomPrices[roomType] * (float)gDays * (float)gRooms
        + (float)gBeds * 50.0f;

    /* Bill preview + confirm button */
    ImGui::BeginChild("BillPreview", ImVec2(0, 100), true);
    ImGui::Columns(2, "billCol", false);

    /* Centre the label+amount block vertically.
       Label line ~13px + gap ~2px + amount line ~26px (scaled 2x) ≈ 41px total
       Child is 100px → top padding ≈ (100 - 41) / 2 ≈ 29px               */
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f);
    ImGui::TextDisabled("ESTIMATED TOTAL:");
    ImGui::SetWindowFontScale(2.0f);
    ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "$ %.2f", totalBill);
    ImGui::SetWindowFontScale(1.0f);

    ImGui::NextColumn();
    /* Centre the 50px button inside the 100px child → 25px top padding */
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 25.0f);

    if (ImGui::Button("CONFIRM RESERVATION", ImVec2(-1, 50))) {
        if (strlen(gName) > 0) {
            Reservation newRes{};
            newRes.id = (int)reservations.size() + 1;

            std::string userData = session.currentUser.firstName + " " +
                session.currentUser.lastName + " (" +
                session.currentUser.email + ")";
            strncpy_s(newRes.customerName, userData.c_str(),
                sizeof(newRes.customerName) - 1);
            strncpy_s(newRes.phone, gPhone, sizeof(newRes.phone) - 1);
            newRes.tableNumber = (int)reservations.size() + 101;
            newRes.guests = gDays;
            strncpy_s(newRes.roomType, roomTypes[roomType], sizeof(newRes.roomType) - 1);
            strncpy_s(newRes.packageName, "All Incl.", sizeof(newRes.packageName) - 1);
            newRes.totalBill = totalBill;
            strncpy_s(newRes.checkIn, startDate, sizeof(newRes.checkIn) - 1);
            strncpy_s(newRes.checkOut, endDate, sizeof(newRes.checkOut) - 1);
            strncpy_s(newRes.checkInTime, chosenCheckInTime, sizeof(newRes.checkInTime) - 1);

            addReservation(newRes);
            gPhone[0] = '\0'; gEmail[0] = '\0';
            gDays = 1; gGuests = 1; gRooms = 1; gBeds = 1; gCheckInOffset = 0;
        }
    }

    ImGui::EndChild();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::End();
}

// ─── App loop ─────────────────────────────────────────────────────────────────

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

        if (!getAuthSession().loggedIn) {
            renderAuthUI();
        }
        else {
            if (getAuthSession().currentUser.role == UserRole::Admin)
                renderUI();
            else
                renderGuestUI();
        }

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