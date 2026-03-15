#ifndef DOCTOR_H
#define DOCTOR_H

#include "../config.h"

typedef struct {
    int  doctor_id;
    char name[NAME_LEN];
    char specialization[SPEC_LEN];
    char phone[PHONE_LEN];
    char available_days[DAYS_LEN];  /* e.g. "Mon,Tue,Wed" */
    int  is_active;
} Doctor;

void addDoctor(void);
void searchDoctor(void);
void updateDoctor(void);
void deleteDoctor(void);
void listAllDoctors(void);

int  doctorExists(int doctor_id);
int  getDoctorById(int doctor_id, Doctor *out);

#endif /* DOCTOR_H */
