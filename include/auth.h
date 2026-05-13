#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <vector>

// Roles a user can have
enum class UserRole {
    Guest,
    Admin
};

// A registered user account
struct UserAccount {
    std::string firstName;
    std::string lastName;
    std::string email;
    std::string passwordHash; // stores a simple hash of the password
    UserRole    role;
};

// Holds the result of an auth session
struct AuthSession {
    bool        loggedIn = false;
    UserAccount currentUser;
};

// Call once at startup - seeds a default admin account
void        initAuth();

// Returns the global auth session (loggedIn = true after successful login)
AuthSession& getAuthSession();

// Renders the full Login / Register ImGui window.
// Returns true once the user has successfully logged in.
bool        renderAuthUI();

// Utility: very simple hash (not cryptographic - good enough for a school project)
std::string simpleHash(const std::string& input);

#endif // AUTH_H