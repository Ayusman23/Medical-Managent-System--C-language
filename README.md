# Hospital Management System

A fully functional, terminal-based **Hospital Management System (HMS)** written in **pure C (C99)** with **binary file I/O** for persistent data storage.

---

## 📁 Project Structure

```
hospital_mgmt/
├── main.c                   ← Entry point
├── Makefile                 ← Build system
├── config.h                 ← Global constants & file paths
│
├── frontend/
│   ├── ui.h                 ← UI function prototypes
│   ├── menu.c               ← Main & sub-menus (role-aware)
│   ├── input_handler.c      ← Validated input wrappers
│   ├── display.c            ← Banners, dashboard, about
│   └── auth_ui.c            ← Login & user registration
│
├── backend/
│   ├── patient.h / .c       ← Patient CRUD
│   ├── doctor.h / .c        ← Doctor CRUD
│   ├── appointment.h / .c   ← Appointment scheduling
│   ├── billing.h / .c       ← Billing & invoices
│   ├── ward.h / .c          ← Ward/bed management
│   ├── staff.h / .c         ← Staff CRUD
│   └── utils.h / .c         ← Shared helpers
│
└── database/
    ├── patients.dat
    ├── doctors.dat
    ├── appointments.dat
    ├── billing.dat
    ├── wards.dat
    ├── staff.dat
    └── users.dat
```

---

## 🚀 Building & Running

### Prerequisites

| Platform | Requirement |
|----------|-------------|
| Linux / macOS | `gcc`, `make` |
| Windows | [MinGW-w64](https://www.mingw-w64.org/) or [MSYS2](https://www.msys2.org/) with `gcc` and `make` |

### Build

```bash
# From the hospital_mgmt/ directory:
make

# Windows (MinGW / MSYS2 shell):
mingw32-make
```

### Run

```bash
# Linux / macOS
./hms

# Windows
hms.exe
```

### Clean

```bash
make clean
```

---

## 🔐 Default Login Credentials

| Role  | Username | Password   |
|-------|----------|------------|
| Admin | `admin`  | `admin123` |
| Staff | `staff`  | `staff123` |

> **Admin** has full access to all menus including User Registration, Dashboard, and About.  
> **Staff** has limited access (no add/remove staff, no user registration).

---

## 🏥 Complete Setup & Example Workflow
**To experience the system as a true full-stack Medical Management Application, follow this "Day in the Life" setup:**

1. **Initial Infrastructure Setup**
   - Log in as **Admin** (`admin` / `admin123`).
   - Navigate to `[2] Doctor Management` -> Add your doctors (e.g., General Surgeon). Take note of their given Doctor IDs.
   - Navigate to `[5] Ward / Bed Management` -> Create some wards (e.g., ICU with 10 beds, General with 20 beds).

2. **Register a Walk-in Patient**
   - Head to `[1] Patient Management` -> `[1] Add Patient`.
   - The system automatically assigns a Patient ID.

3. **The Patient Journey: Booking & Admission**
   - Go to `[3] Appointment Scheduling` -> `[1] Book Appointment` linking the Patient ID and Doctor ID.
   - If the doctor orders an operation, go to `[5] Ward / Bed Management` -> `[2] Admit Patient to Ward`. This actively assigns them a bed, which will show up immediately across the system.

4. **Discharge & Billing Checkout**
   - Once recovered, navigate to `[1] Patient Management` -> `[7] Discharge Patient`.
   - Head to the cashier at `[4] Billing & Invoices` -> `[1] Generate Bill`. The system dynamically detects their recent ward admission and alerts you to manually add room charges.
   - Proceed to `[3] Mark Bill as Paid` once they settle the invoice.

5. **End-of-Day Review**
   - Head to `[8] Dashboard` (Admin only) to watch real-time synchronised tallies of the SQLite-like binary databases counting active patients, doctors, generated bills, and occupied beds.

---

## ✅ Modules & Features

### 1. Authentication
- Login screen with username + hashed password (djb2 hash)
- Role-based access control: Admin vs Staff
- Register new users (Admin only)
- Loops back to login after logout

### 2. Patient Management
- Add / Search / Update / Delete (soft delete) patients
- Auto-incremented patient IDs
- Fields: ID, Name, Age, Gender, Blood Group, Phone, Address, Admission Date, Ward No
- Search by ID or name substring

### 3. Doctor Management
- Full CRUD for doctors
- Search by ID, name, or specialization
- Fields: ID, Name, Specialization, Phone, Available Days

### 4. Appointment Scheduling
- Book / Cancel appointments
- View appointments filtered by doctor or patient
- Status tracking: Scheduled / Cancelled / Completed

### 5. Billing & Invoices
- Generate itemised bills (room + medicine + doctor fee)
- Auto-computed total
- Formatted invoice printout on terminal
- Mark bills as Paid / Unpaid
- Grand total payroll summary

### 6. Ward / Bed Management
- Add wards with type (General/ICU/Pediatric/etc.) and bed capacity
- Admit patients to wards (respects bed capacity)
- Discharge patients (shifts array)
- View per-ward or all-ward status

### 7. Staff Management
- Add / Update / Remove staff
- Search by ID, name, or department
- Total payroll summary
- Salary tracking

---

## 🛠 Technical Details

| Feature | Implementation |
|---------|---------------|
| Language | Pure C (C99) |
| Storage | Binary files via `fread()` / `fwrite()` |
| IDs | Auto-incremented (scan file for max ID) |
| Soft Delete | `is_active` flag in every struct |
| Passwords | djb2 hash stored as `unsigned int` |
| Screen clear | `cls` (Windows) / `clear` (POSIX) via `#ifdef _WIN32` |
| ANSI colours | Enabled via `SetConsoleMode` on Windows 10+ |
| Input safety | `fgets` + flush loop + `strTrimWhitespace` |
| Build | GNU Make, single executable output |

---

## 📝 Notes

- All `*.dat` files are created automatically on first run inside `database/`.
- There are **no external dependencies** — only the C standard library.
- Tested with GCC 12+ on Windows (MinGW-w64) and Ubuntu 22.04.
