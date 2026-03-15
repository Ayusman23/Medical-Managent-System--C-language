#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include "../config.h"

typedef struct {
    int  appt_id;
    int  patient_id;
    int  doctor_id;
    char date[DATE_LEN];
    char time_slot[TIME_LEN];   /* e.g. "09:00" */
    char status[STATUS_LEN];    /* Scheduled / Cancelled / Completed */
    int  is_active;
} Appointment;

void bookAppointment(void);
void cancelAppointment(void);
void viewAppointmentsByDoctor(void);
void viewAppointmentsByPatient(void);
void listAllAppointments(void);
void viewTodayAppointments(void);   /* show only today's schedule */

#endif /* APPOINTMENT_H */
