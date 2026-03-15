#ifndef CONFIG_H
#define CONFIG_H

/* ============================================================
   Hospital Management System - Global Configuration
   ============================================================ */

/* Database file paths */
#define DB_PATIENTS     "database/patients.dat"
#define DB_DOCTORS      "database/doctors.dat"
#define DB_APPOINTMENTS "database/appointments.dat"
#define DB_BILLING      "database/billing.dat"
#define DB_WARDS        "database/wards.dat"
#define DB_STAFF        "database/staff.dat"
#define DB_USERS        "database/users.dat"

/* Record limits */
#define MAX_PATIENTS     1000
#define MAX_DOCTORS      200
#define MAX_APPOINTMENTS 5000
#define MAX_BILLS        5000
#define MAX_WARDS        50
#define MAX_STAFF        500
#define MAX_USERS        100
#define MAX_BEDS_PER_WARD 30

/* String field sizes */
#define NAME_LEN         60
#define PHONE_LEN        20
#define ADDR_LEN         120
#define DATE_LEN         12   /* YYYY-MM-DD */
#define TIME_LEN         10   /* HH:MM      */
#define SPEC_LEN         50
#define DAYS_LEN         50
#define BLOODGRP_LEN     6
#define GENDER_LEN       10
#define ROLE_LEN         40
#define DEPT_LEN         40
#define STATUS_LEN       20
#define WTYPE_LEN        30
#define USERNAME_LEN     32
#define PASS_LEN         64

/* Roles */
#define ROLE_ADMIN  1
#define ROLE_STAFF  2

/* Colors (ANSI) – no-op on windows without VT100 enabled */
#define CLR_RESET  "\033[0m"
#define CLR_BOLD   "\033[1m"
#define CLR_RED    "\033[31m"
#define CLR_GREEN  "\033[32m"
#define CLR_YELLOW "\033[33m"
#define CLR_CYAN   "\033[36m"
#define CLR_BLUE   "\033[34m"
#define CLR_MAGENTA "\033[35m"
#define CLR_WHITE  "\033[37m"

/* Cross-platform clear screen */
#ifdef _WIN32
  #define CLEAR_SCREEN() system("cls")
#else
  #define CLEAR_SCREEN() system("clear")
#endif

#endif /* CONFIG_H */
