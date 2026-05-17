#define _CRT_SECURE_NO_WARNINGS
#include "auth.h"
#include "imgui.h"
#include <cstring>
#include <algorithm>

static std::vector<UserAccount> s_accounts;
static AuthSession              s_session;

std::string simpleHash(const std::string& input) {
	unsigned long hash = 5381;
	for (char c : input)
		hash = ((hash << 5) + hash) + (unsigned char)c;
	return std::to_string(hash);
}

void initAuth() {
	UserAccount admin;
	admin.firstName = "Admin";
	admin.lastName = "User";
	admin.email = "admin@hotel.com";
	admin.passwordHash = simpleHash("admin123");
	admin.role = UserRole::Admin;
	s_accounts.push_back(admin);
}

AuthSession& getAuthSession() {
	return s_session;
}

static bool isValidEmail(const char* email) {
	const char* at = strchr(email, '@');
	if (!at || at == email) return false;
	const char* dot = strchr(at, '.');
	return dot && dot[1] != '\0';
}

static UserAccount* findAccount(const std::string& email) {
	for (auto& acc : s_accounts)
		if (acc.email == email)
			return &acc;
	return nullptr;
}

static int  s_tab = 0; // 0 = Login, 1 = Register
static char s_loginEmail[128] = "";
static char s_loginPw[128] = "";
static char s_regFirst[64] = "";
static char s_regLast[64] = "";
static char s_regEmail[128] = "";
static char s_regPw[128] = "";
static char s_regPw2[128] = "";
static int  s_regRole = 0; // 0 = Guest, 1 = Admin
static char s_errorMsg[256] = "";
static char s_successMsg[256] = "";
static bool s_showLoginPw = false;
static bool s_showRegPw = false;
static bool s_showRegPw2 = false;

static void clearMessages() {
	s_errorMsg[0] = '\0';
	s_successMsg[0] = '\0';
}

void logoutAuth() {
	s_session.loggedIn = false;
	s_session.currentUser = UserAccount{};
	s_loginEmail[0] = '\0';
	s_loginPw[0] = '\0';
	s_regFirst[0] = '\0';
	s_regLast[0] = '\0';
	s_regEmail[0] = '\0';
	s_regPw[0] = '\0';
	s_regPw2[0] = '\0';
	s_regRole = 0;
	s_errorMsg[0] = '\0';
	s_successMsg[0] = '\0';
	s_tab = 0;
}

static int passwordStrength(const char* pw) {
	int score = 0;
	int len = (int)strlen(pw);
	if (len >= 8)  score++;
	bool hasUpper = false, hasDigit = false, hasSpecial = false;
	for (int i = 0; i < len; i++) {
		if (isupper((unsigned char)pw[i]))  hasUpper = true;
		if (isdigit((unsigned char)pw[i]))  hasDigit = true;
		if (!isalnum((unsigned char)pw[i])) hasSpecial = true;
	}
	if (hasUpper)   score++;
	if (hasDigit)   score++;
	if (hasSpecial) score++;
	return score;
}

bool renderAuthUI() {
	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
		ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(440, 0), ImGuiCond_Always); // auto height

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoTitleBar;

	ImGui::Begin("##AuthWindow", nullptr, flags);
	ImGui::Spacing();
	ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Hotel Reservation System").x) * 0.5f);
	ImGui::TextUnformatted("Hotel Reservation System");
	ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Management Portal").x) * 0.5f);
	ImGui::TextDisabled("Management Portal");
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	if (ImGui::BeginTabBar("AuthTabs")) {
		if (ImGui::BeginTabItem("Sign In")) {
			if (s_tab != 0) { clearMessages(); s_tab = 0; }
			ImGui::Spacing();
			if (s_successMsg[0]) {
				ImGui::TextColored(ImVec4(0.1f, 0.7f, 0.3f, 1.0f), "%s", s_successMsg);
				ImGui::Spacing();
			}
			if (s_errorMsg[0]) {
				ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.0f), "%s", s_errorMsg);
				ImGui::Spacing();
			}
			ImGui::TextDisabled("EMAIL");
			ImGui::SetNextItemWidth(-1);
			ImGui::InputText("##LoginEmail", s_loginEmail, sizeof(s_loginEmail));
			ImGui::Spacing();
			ImGui::TextDisabled("PASSWORD");
			ImGui::SetNextItemWidth(-FLT_MIN - 60);
			ImGuiInputTextFlags pwFlags = s_showLoginPw ? 0 : ImGuiInputTextFlags_Password;
			ImGui::InputText("##LoginPw", s_loginPw, sizeof(s_loginPw), pwFlags);
			ImGui::SameLine();
			if (ImGui::SmallButton(s_showLoginPw ? "Hide" : "Show"))
				s_showLoginPw = !s_showLoginPw;
			ImGui::Spacing();
			ImGui::SetNextItemWidth(-1);
			if (ImGui::Button("Sign In", ImVec2(-1, 36))) {
				clearMessages();
				if (!isValidEmail(s_loginEmail)) {
					strcpy(s_errorMsg, "Please enter a valid email address.");
				}
				else if (strlen(s_loginPw) == 0) {
					strcpy(s_errorMsg, "Password is required.");
				}
				else {
					UserAccount* acc = findAccount(std::string(s_loginEmail));
					if (!acc || acc->passwordHash != simpleHash(std::string(s_loginPw))) {
						strcpy(s_errorMsg, "Incorrect email or password.");
					}
					else {
						s_session.loggedIn = true;
						s_session.currentUser = *acc;
						ImGui::EndTabItem();
						ImGui::EndTabBar();
						ImGui::End();
						return true; // signal: auth done
					}
				}
			}

			ImGui::Spacing();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Create Account")) {
			if (s_tab != 1) { clearMessages(); s_tab = 1; }
			ImGui::Spacing();

			if (s_successMsg[0]) {
				ImGui::TextColored(ImVec4(0.1f, 0.7f, 0.3f, 1.0f), "%s", s_successMsg);
				ImGui::Spacing();
			}
			if (s_errorMsg[0]) {
				ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.0f), "%s", s_errorMsg);
				ImGui::Spacing();
			}
			if (ImGui::BeginTable("NameTable", 2, ImGuiTableFlags_None)) {
				ImGui::TableSetupColumn("First", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Last", ImGuiTableColumnFlags_WidthStretch);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0); ImGui::TextDisabled("FIRST NAME");
				ImGui::TableSetColumnIndex(1); ImGui::TextDisabled("LAST NAME");

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::SetNextItemWidth(-1);
				ImGui::InputText("##RegFirst", s_regFirst, sizeof(s_regFirst));
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(-1);
				ImGui::InputText("##RegLast", s_regLast, sizeof(s_regLast));

				ImGui::EndTable();
			}
			ImGui::Spacing();
			ImGui::TextDisabled("EMAIL");
			ImGui::SetNextItemWidth(-1);
			ImGui::InputText("##RegEmail", s_regEmail, sizeof(s_regEmail));
			ImGui::Spacing();
			ImGui::TextDisabled("PASSWORD");
			ImGui::SetNextItemWidth(-FLT_MIN - 60);
			ImGuiInputTextFlags pwFlags = s_showRegPw ? 0 : ImGuiInputTextFlags_Password;
			ImGui::InputText("##RegPw", s_regPw, sizeof(s_regPw), pwFlags);
			ImGui::SameLine();
			if (ImGui::SmallButton(s_showRegPw ? "Hide##1" : "Show##1"))
				s_showRegPw = !s_showRegPw;
			if (strlen(s_regPw) > 0) {
				int strength = passwordStrength(s_regPw);
				const char* labels[] = { "", "Weak", "Fair", "Good", "Strong" };
				ImVec4      colors[] = {
					{0,0,0,0},
					{0.85f, 0.25f, 0.25f, 1.0f},
					{0.95f, 0.65f, 0.15f, 1.0f},
					{0.35f, 0.65f, 0.95f, 1.0f},
					{0.10f, 0.70f, 0.40f, 1.0f}
				};
				float segW = (ImGui::GetContentRegionAvail().x - 12.0f) / 4.0f;
				for (int i = 0; i < 4; i++) {
					ImVec4 col = (i < strength) ? colors[strength] : ImVec4(0.25f, 0.25f, 0.25f, 0.3f);
					ImGui::PushStyleColor(ImGuiCol_PlotHistogram, col);
					ImGui::ProgressBar(1.0f, ImVec2(segW, 4.0f), "");
					ImGui::PopStyleColor();
					if (i < 3) ImGui::SameLine(0, 3);
				}
				ImGui::SameLine();
				ImGui::TextColored(colors[strength], " %s", labels[strength]);
			}
			ImGui::Spacing();
			ImGui::TextDisabled("CONFIRM PASSWORD");
			ImGui::SetNextItemWidth(-FLT_MIN - 60);
			ImGuiInputTextFlags pw2Flags = s_showRegPw2 ? 0 : ImGuiInputTextFlags_Password;
			ImGui::InputText("##RegPw2", s_regPw2, sizeof(s_regPw2), pw2Flags);
			ImGui::SameLine();
			if (ImGui::SmallButton(s_showRegPw2 ? "Hide##2" : "Show##2"))
				s_showRegPw2 = !s_showRegPw2;
			ImGui::Spacing();
			if (ImGui::Button("Create Account", ImVec2(-1, 36))) {
				clearMessages();
				bool ok = true;
				if (strlen(s_regFirst) == 0) { strcpy(s_errorMsg, "First name is required.");       ok = false; }
				else if (strlen(s_regLast) == 0) { strcpy(s_errorMsg, "Last name is required.");    ok = false; }
				else if (!isValidEmail(s_regEmail)) { strcpy(s_errorMsg, "Enter a valid email.");   ok = false; }
				else if (findAccount(s_regEmail)) { strcpy(s_errorMsg, "Email already registered."); ok = false; }
				else if (strlen(s_regPw) < 8) { strcpy(s_errorMsg, "Password must be 8+ chars.");  ok = false; }
				else if (strcmp(s_regPw, s_regPw2) != 0) { strcpy(s_errorMsg, "Passwords do not match."); ok = false; }

				if (ok) {
					UserAccount newAcc;
					newAcc.firstName = s_regFirst;
					newAcc.lastName = s_regLast;
					newAcc.email = s_regEmail;
					newAcc.passwordHash = simpleHash(std::string(s_regPw));
					newAcc.role = UserRole::Guest;
					s_accounts.push_back(newAcc);
					s_session.loggedIn = true;
					s_session.currentUser = s_accounts.back();
					ImGui::EndTabItem();
					ImGui::EndTabBar();
					ImGui::End();
					return true; // signal: auth done
				}
			}

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::Spacing();
	ImGui::End();
	return false; // not logged in yet
}