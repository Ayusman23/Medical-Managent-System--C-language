#include <stdio.h>
#include <stdlib.h>
#include "../config.h"
#include "../backend/utils.h"
#include "../backend/patient.h"
#include "../backend/doctor.h"
#include "../backend/appointment.h"
#include "../backend/billing.h"
#include "../backend/ward.h"
#include "../backend/staff.h"
#include "ui.h"

/* ============================================================
   Print styled sub-menu header
   ============================================================ */
static void menuHeader(const char *title, const char *color) {
    CLEAR_SCREEN();
    printf("%s" CLR_BOLD, color);
    printf("\n  ╔══════════════════════════════════════╗\n");
    printf("  ║  %-36s║\n", title);
    printf("  ╚══════════════════════════════════════╝\n" CLR_RESET);
}

/* ============================================================
   Patient Menu
   ============================================================ */
void showPatientMenu(void) {
    int choice;
    do {
        menuHeader("PATIENT MANAGEMENT", CLR_GREEN);
        printf("  1. Add Patient\n");
        printf("  2. Search Patient\n");
        printf("  3. Update Patient\n");
        printf("  4. Delete Patient\n");
        printf("  5. List All Patients\n");
        printf("  6. View Admitted Patients\n");
        printf("  7. Discharge Patient\n");
        printf("  0. Back\n");
        printf("\n  Choice: "); fflush(stdout);
        choice = inputInt(0, 7);
        switch (choice) {
            case 1: addPatient();           break;
            case 2: searchPatient();        break;
            case 3: updatePatient();        break;
            case 4: deletePatient();        break;
            case 5: listAllPatients();      break;
            case 6: viewAdmittedPatients(); break;
            case 7: dischargePatient();     break;
        }
    } while (choice != 0);
}

/* ============================================================
   Doctor Menu
   ============================================================ */
void showDoctorMenu(void) {
    int choice;
    do {
        menuHeader("DOCTOR MANAGEMENT", CLR_BLUE);
        printf("  1. Add Doctor\n");
        printf("  2. Search Doctor\n");
        printf("  3. Update Doctor\n");
        printf("  4. Delete Doctor\n");
        printf("  5. List All Doctors\n");
        printf("  0. Back\n");
        printf("\n  Choice: "); fflush(stdout);
        choice = inputInt(0, 5);
        switch (choice) {
            case 1: addDoctor();       break;
            case 2: searchDoctor();    break;
            case 3: updateDoctor();    break;
            case 4: deleteDoctor();    break;
            case 5: listAllDoctors();  break;
        }
    } while (choice != 0);
}

/* ============================================================
   Appointment Menu
   ============================================================ */
void showAppointmentMenu(void) {
    int choice;
    do {
        menuHeader("APPOINTMENT SCHEDULING", CLR_YELLOW);
        printf("  1. Book Appointment\n");
        printf("  2. Cancel / Update Appointment\n");
        printf("  3. View by Doctor\n");
        printf("  4. View by Patient\n");
        printf("  5. List All Appointments\n");
        printf("  6. Today's Appointments\n");
        printf("  0. Back\n");
        printf("\n  Choice: "); fflush(stdout);
        choice = inputInt(0, 6);
        switch (choice) {
            case 1: bookAppointment();              break;
            case 2: cancelAppointment();            break;
            case 3: viewAppointmentsByDoctor();     break;
            case 4: viewAppointmentsByPatient();    break;
            case 5: listAllAppointments();          break;
            case 6: viewTodayAppointments();        break;
        }
    } while (choice != 0);
}

/* ============================================================
   Billing Menu
   ============================================================ */
void showBillingMenu(void) {
    int choice;
    do {
        menuHeader("BILLING & INVOICES", CLR_MAGENTA);
        printf("  1. Generate Bill\n");
        printf("  2. View Bill (Invoice)\n");
        printf("  3. Mark Bill as Paid\n");
        printf("  4. List All Bills\n");
        printf("  0. Back\n");
        printf("\n  Choice: "); fflush(stdout);
        choice = inputInt(0, 4);
        switch (choice) {
            case 1: generateBill();   break;
            case 2: viewBill();       break;
            case 3: markPaid();       break;
            case 4: listAllBills();   break;
        }
    } while (choice != 0);
}

/* ============================================================
   Ward Menu
   ============================================================ */
void showWardMenu(void) {
    int choice;
    do {
        menuHeader("WARD / BED MANAGEMENT", CLR_CYAN);
        printf("  1. Add New Ward\n");
        printf("  2. Admit Patient to Ward\n");
        printf("  3. Discharge Patient from Ward\n");
        printf("  4. View Ward Status\n");
        printf("  5. List All Wards\n");
        printf("  0. Back\n");
        printf("\n  Choice: "); fflush(stdout);
        choice = inputInt(0, 5);
        switch (choice) {
            case 1: addWard();              break;
            case 2: admitToWard();          break;
            case 3: dischargeFromWard();    break;
            case 4: viewWardStatus();       break;
            case 5: listAllWards();         break;
        }
    } while (choice != 0);
}

/* ============================================================
   Staff Menu (restricted for non-admin)
   ============================================================ */
void showStaffMenu(int role) {
    int choice;
    do {
        menuHeader("STAFF MANAGEMENT", CLR_WHITE);
        printf("  1. List All Staff\n");
        printf("  2. Search Staff\n");
        if (role == ROLE_ADMIN) {
            printf("  3. Add Staff\n");
            printf("  4. Update Staff\n");
            printf("  5. Remove Staff\n");
        }
        printf("  0. Back\n");
        printf("\n  Choice: "); fflush(stdout);
        int max = (role == ROLE_ADMIN) ? 5 : 2;
        choice = inputInt(0, max);
        switch (choice) {
            case 1: listStaff();    break;
            case 2: searchStaff();  break;
            case 3: if (role == ROLE_ADMIN) addStaff();     break;
            case 4: if (role == ROLE_ADMIN) updateStaff();  break;
            case 5: if (role == ROLE_ADMIN) removeStaff();  break;
        }
    } while (choice != 0);
}

/* ============================================================
   Main Menu  (role-aware)
   ============================================================ */
void showMainMenu(int role) {
    int choice;
    do {
        CLEAR_SCREEN();
        printf(CLR_BOLD CLR_CYAN);
        printf("\n  ╔══════════════════════════════════════════╗\n");
        printf("  ║       HOSPITAL MANAGEMENT SYSTEM         ║\n");
        printf("  ║       Role: %-30s║\n",
               role == ROLE_ADMIN ? "Administrator" : "Staff");
        printf("  ╠══════════════════════════════════════════╣\n");
        printf("  ║  1. Patient Management                   ║\n");
        printf("  ║  2. Doctor Management                    ║\n");
        printf("  ║  3. Appointment Scheduling               ║\n");
        printf("  ║  4. Billing & Invoices                   ║\n");
        printf("  ║  5. Ward / Bed Management                ║\n");
        printf("  ║  6. Staff Management                     ║\n");
        if (role == ROLE_ADMIN) {
            printf("  ║  7. Register New User                    ║\n");
            printf("  ║  8. Dashboard                            ║\n");
            printf("  ║  9. About                                ║\n");
        }
        printf("  ║  0. Logout                               ║\n");
        printf("  ╚══════════════════════════════════════════╝\n" CLR_RESET);
        printf("  Choice: "); fflush(stdout);

        int max = (role == ROLE_ADMIN) ? 9 : 6;
        choice = inputInt(0, max);

        switch (choice) {
            case 1: showPatientMenu();          break;
            case 2: showDoctorMenu();           break;
            case 3: showAppointmentMenu();      break;
            case 4: showBillingMenu();          break;
            case 5: showWardMenu();             break;
            case 6: showStaffMenu(role);        break;
            case 7: if (role==ROLE_ADMIN) showRegistrationScreen(); break;
            case 8: if (role==ROLE_ADMIN) showDashboard();          break;
            case 9: if (role==ROLE_ADMIN) showAbout();              break;
            case 0: printf(CLR_YELLOW "  Logging out...\n" CLR_RESET); break;
        }
    } while (choice != 0);
}
