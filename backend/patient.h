#ifndef PATIENT_H
#define PATIENT_H

#include "../config.h"

/* ============================================================
   Patient record structure
   ============================================================ */
typedef struct {
    int  patient_id;
    char name[NAME_LEN];
    int  age;
    char gender[GENDER_LEN];
    char blood_group[BLOODGRP_LEN];
    char phone[PHONE_LEN];
    char address[ADDR_LEN];
    char admitted_date[DATE_LEN];
    int  ward_no;           /* 0 = not admitted to any ward */
    int  is_active;         /* 0 = deleted (soft delete)    */
} Patient;

/* CRUD */
void addPatient(void);
void searchPatient(void);
void updatePatient(void);
void deletePatient(void);
void listAllPatients(void);

/* Internal helpers used by other modules */
int  patientExists(int patient_id);
int  getPatientById(int patient_id, Patient *out);
int  updatePatientWardNo(int patient_id, int ward_no); /* called by ward.c */
void dischargePatient(void);    /* marks patient as discharged (ward_no=0) */
void viewAdmittedPatients(void); /* list patients currently in a ward       */

#endif /* PATIENT_H */
