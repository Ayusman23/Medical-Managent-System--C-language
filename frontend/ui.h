#ifndef UI_H
#define UI_H

/* ============================================================
   Frontend UI - function prototypes
   ============================================================ */

/* auth_ui.c */
int  showLoginScreen(int *role_out);   /* returns 1 on success, 0 on failure */
void showRegistrationScreen(void);

/* menu.c */
void showMainMenu(int role);
void showPatientMenu(void);
void showDoctorMenu(void);
void showAppointmentMenu(void);
void showBillingMenu(void);
void showWardMenu(void);
void showStaffMenu(int role);

/* display.c */
void showWelcomeBanner(void);
void showDashboard(void);
void showAbout(void);

/* input_handler.c */
void getValidatedString(const char *prompt, char *dest, int maxlen, int allow_empty);
int  getValidatedInt(const char *prompt, int min, int max);
float getValidatedFloat(const char *prompt, float min, float max);
void getValidatedDate(const char *prompt, char *dest);
void getValidatedPhone(const char *prompt, char *dest);

#endif /* UI_H */
