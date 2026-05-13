# 🗓️ Reservation Management System

A C++ desktop application for managing restaurant reservations, built with **Dear ImGui** and **GLFW/OpenGL**. Supports sorting, searching, and recursive guest counting through a clean graphical interface.

---

## 📋 Project Description

This application lets users manage a list of reservations through a menu-based GUI. It supports:

- **Sorting** reservations alphabetically by customer name (Bubble Sort)
- **Searching** for a reservation by customer name (Linear Search)
- **Recursively calculating** the total number of guests across all reservations
- **Viewing** all reservations in a structured table with ID, Name, Table, and Guest columns

Built using a **three-tier architecture** (Presentation → Logic → Data) and developed following the **Scrum methodology**.

---

## 👥 Team Members & Roles

| Name | Role |
|------|------|
| Antonio Ivanov | Scrum Master / Documenter |
| Ivan Lapchev | Back-End Developer |
| Presiqn Tenev | Back-End Developer |
| Loris Zadikyan | Front-End Developer |

---

## 🏗️ Architecture

The project follows a strict **Three-Tier Architecture**:

```
┌──────────────────────────────────────┐
│         Presentation Layer           │
│  presentation.h / presentation.cpp   │
│  Renders GUI via Dear ImGui          │
└──────────────────┬───────────────────┘
                   │ calls
┌──────────────────▼───────────────────┐
│            Logic Layer               │
│      logic.h / logic.cpp             │
│  Sorting, Searching, Recursion       │
└──────────────────┬───────────────────┘
                   │ calls
┌──────────────────▼───────────────────┐
│             Data Layer               │
│       data.h / data.cpp              │
│  Reservation struct, storage, I/O    │
└──────────────────────────────────────┘
```

- **Presentation** — renders the ImGui GUI, handles user input, calls only Logic layer functions
- **Logic** — implements all algorithms; calls only Data layer functions
- **Data** — defines the `Reservation` struct, holds the in-memory database, exposes `getReservations()`, `addReservation()`, `initData()`


---

## 📁 Project Structure

```
null-terminators-reservation-system/
├── src/
│   ├── main.cpp            # Entry point
│   ├── presentation.cpp    # GUI rendering (ImGui + GLFW)
│   ├── logic.cpp           # Sorting, searching, recursion
│   └── data.cpp            # Data storage and struct definitions
├── include/
│   ├── presentation.h
│   ├── logic.h
│   └── data.h
├── libs/
│   └── imGui/              # Dear ImGui source files
├── Documentation/          # Architecture diagrams, sprint reports
└── README.md
```

---

## ⚙️ Setup & Installation

### Prerequisites

- [Visual Studio 2022](https://visualstudio.microsoft.com/) with the **Desktop development with C++** workload
- [GLFW](https://www.glfw.org/download.html) — windowing and OpenGL context (64-bit Windows binaries)
- Dear ImGui — already included in `libs/imGui/`
- OpenGL — included with Windows / Visual Studio

### Building & Running

1. Open `reservationManagementSystem.slnx` in Visual Studio 2022
2. Set the configuration to **Debug** and platform to **x64**
3. Press **Ctrl+Shift+B** to build
4. Press **F5** to run

---

## 🚀 How to Use

Once the application launches, the **"Reservation Management System"** window will appear:

| Feature | How to use |
|--------|------------|
| Sort by Name | Click the **"Sort by Name"** button |
| Search | Type a customer name in **"Search Customer"** and click **"Find"** — the matching row highlights green |
| Total Guests | Displayed automatically at the top, calculated recursively |
| View Reservations | All reservations are shown in the table with ID, Name, Table, and Guests columns |

---

## 📦 Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| [Dear ImGui](https://github.com/ocornut/imgui) | Included in `libs/imGui/` | GUI rendering |
| [GLFW](https://www.glfw.org/) | 3.x (64-bit) | Window creation and OpenGL context |
| OpenGL | System | Graphics rendering backend |
