#include <stdio.h>
#include "config.h"
#include "backend/patient.h"
#include "backend/doctor.h"
#include "backend/appointment.h"
#include "backend/billing.h"
#include "backend/ward.h"
#include "backend/staff.h"

int main() {
    printf("Patient: %zu\n", sizeof(Patient));
    printf("Doctor: %zu\n", sizeof(Doctor));
    printf("Appointment: %zu\n", sizeof(Appointment));
    printf("Bill: %zu\n", sizeof(Bill));
    printf("Ward: %zu\n", sizeof(Ward));
    printf("Staff: %zu\n", sizeof(Staff));
    return 0;
}
