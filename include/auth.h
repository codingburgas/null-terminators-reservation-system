#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <vector>

enum class UserRole {
    Guest,
    Admin
};

struct UserAccount {
    std::string firstName;
    std::string lastName;
    std::string email;
    std::string passwordHash;
    UserRole    role = UserRole::Guest;
};

struct AuthSession {
    bool        loggedIn = false;
    UserAccount currentUser;
};

void         initAuth();
AuthSession& getAuthSession();
void         logoutAuth();
bool         renderAuthUI();
std::string  simpleHash(const std::string& input);

#endif // AUTH_H
