import struct
import os
import json
from flask import Flask, jsonify, request, send_from_directory
from flask_cors import CORS

app = Flask(__name__, static_folder='web')
CORS(app)

# Struct Formats
PATIENT_FMT = '<i 60s i 10s 6s 20s 120s 12s i i'
PATIENT_SIZE = 244

DOCTOR_FMT = '<i 60s 50s 20s 50s i'
DOCTOR_SIZE = 188

APPT_FMT = '<i i i 12s 10s 20s 2x i'
APPT_SIZE = 60

BILL_FMT = '<i i f f f f i 12s i'
BILL_SIZE = 44

WARD_FMT = '<i 30s 2x i i 30i i'
WARD_SIZE = 168

STAFF_FMT = '<i 60s 40s 40s 20s f i'
STAFF_SIZE = 172

DB_DIR = 'database'

def decode_str(b):
    return b.split(b'\x00')[0].decode('utf-8', errors='ignore')

def encode_str(s, length):
    b = s.encode('utf-8')
    return b + b'\x00' * (length - len(b))

def read_records(filename, record_fmt, record_size, parse_fn):
    filepath = os.path.join(DB_DIR, filename)
    if not os.path.exists(filepath):
        return []
    records = []
    with open(filepath, 'rb') as f:
        while True:
            chunk = f.read(record_size)
            if not chunk or len(chunk) < record_size:
                break
            record = struct.unpack(record_fmt, chunk)
            parsed = parse_fn(record)
            if parsed.get('is_active', 1) == 1:
                records.append(parsed)
    return records

def write_records(filename, record_fmt, record_size, serialize_fn, records_to_upsert):
    filepath = os.path.join(DB_DIR, filename)
    all_records = []
    # Read all raw records first (including inactive)
    if os.path.exists(filepath):
        with open(filepath, 'rb') as f:
            while True:
                chunk = f.read(record_size)
                if not chunk or len(chunk) < record_size:
                    break
                all_records.append(struct.unpack(record_fmt, chunk))
    
    # Update or Add
    for upsert in records_to_upsert:
        updated = False
        new_tuple = serialize_fn(upsert)
        # ID is usually the first element in the tuple
        for i, r in enumerate(all_records):
            if r[0] == new_tuple[0]:
                all_records[i] = new_tuple
                updated = True
                break
        if not updated:
            all_records.append(new_tuple)
            
    # Write back
    with open(filepath, 'wb') as f:
        for r in all_records:
            f.write(struct.pack(record_fmt, *r))

# -- Patients --
def parse_patient(r):
    return {
        'id': r[0],
        'name': decode_str(r[1]),
        'age': r[2],
        'gender': decode_str(r[3]),
        'blood_group': decode_str(r[4]),
        'phone': decode_str(r[5]),
        'address': decode_str(r[6]),
        'admitted_date': decode_str(r[7]),
        'ward_no': r[8],
        'is_active': r[9]
    }

def serialize_patient(p):
    return (
        p['id'],
        encode_str(p['name'], 60),
        p['age'],
        encode_str(p['gender'], 10),
        encode_str(p['blood_group'], 6),
        encode_str(p['phone'], 20),
        encode_str(p['address'], 120),
        encode_str(p['admitted_date'], 12),
        p.get('ward_no', 0),
        p.get('is_active', 1)
    )

@app.route('/api/patients', methods=['GET'])
def get_patients():
    patients = read_records('patients.dat', PATIENT_FMT, PATIENT_SIZE, parse_patient)
    return jsonify(patients)

@app.route('/api/patients', methods=['POST'])
def add_patient():
    data = request.json
    all_p = read_records('patients.dat', PATIENT_FMT, PATIENT_SIZE, parse_patient)
    new_id = max([p['id'] for p in all_p] + [0]) + 1
    data['id'] = new_id
    data['is_active'] = 1
    write_records('patients.dat', PATIENT_FMT, PATIENT_SIZE, serialize_patient, [data])
    return jsonify({'success': True, 'patient': data})

@app.route('/api/patients/<int:pid>', methods=['DELETE'])
def delete_patient(pid):
    filepath = os.path.join(DB_DIR, 'patients.dat')
    if not os.path.exists(filepath): return jsonify({'success':False})
    all_records = []
    with open(filepath, 'rb') as f:
        while True:
            chunk = f.read(PATIENT_SIZE)
            if not chunk or len(chunk) < PATIENT_SIZE: break
            all_records.append(struct.unpack(PATIENT_FMT, chunk))
    
    with open(filepath, 'wb') as f:
        for r in all_records:
            if r[0] == pid:
                # set is_active = 0
                r = list(r)
                r[9] = 0
                r = tuple(r)
            f.write(struct.pack(PATIENT_FMT, *r))
    return jsonify({'success': True})

# -- Doctors --
def parse_doctor(r):
    return {
        'id': r[0],
        'name': decode_str(r[1]),
        'specialization': decode_str(r[2]),
        'phone': decode_str(r[3]),
        'available_days': decode_str(r[4]),
        'is_active': r[5]
    }

def serialize_doctor(d):
    return (
        d['id'],
        encode_str(d['name'], 60),
        encode_str(d['specialization'], 50),
        encode_str(d['phone'], 20),
        encode_str(d['available_days'], 50),
        d.get('is_active', 1)
    )

@app.route('/api/doctors', methods=['GET'])
def get_doctors():
    doctors = read_records('doctors.dat', DOCTOR_FMT, DOCTOR_SIZE, parse_doctor)
    return jsonify(doctors)

@app.route('/api/doctors', methods=['POST'])
def add_doctor():
    data = request.json
    all_d = read_records('doctors.dat', DOCTOR_FMT, DOCTOR_SIZE, parse_doctor)
    new_id = max([d['id'] for d in all_d] + [0]) + 1
    data['id'] = new_id
    data['is_active'] = 1
    write_records('doctors.dat', DOCTOR_FMT, DOCTOR_SIZE, serialize_doctor, [data])
    return jsonify({'success': True, 'doctor': data})

@app.route('/api/doctors/<int:did>', methods=['DELETE'])
def delete_doctor(did):
    filepath = os.path.join(DB_DIR, 'doctors.dat')
    if not os.path.exists(filepath): return jsonify({'success':False})
    all_records = []
    with open(filepath, 'rb') as f:
        while True:
            chunk = f.read(DOCTOR_SIZE)
            if not chunk or len(chunk) < DOCTOR_SIZE: break
            all_records.append(struct.unpack(DOCTOR_FMT, chunk))
    
    with open(filepath, 'wb') as f:
        for r in all_records:
            if r[0] == did:
                r = list(r)
                r[5] = 0
                r = tuple(r)
            f.write(struct.pack(DOCTOR_FMT, *r))
    return jsonify({'success': True})

# -- Appointments --
def parse_appt(r):
    return {
        'id': r[0], 'patient_id': r[1], 'doctor_id': r[2],
        'date': decode_str(r[3]), 'time_slot': decode_str(r[4]), 'status': decode_str(r[5]),
        'is_active': r[6]
    }
def serialize_appt(a):
    return (a['id'], a['patient_id'], a['doctor_id'], encode_str(a['date'],12), encode_str(a['time_slot'],10), encode_str(a['status'],20), a.get('is_active',1))

@app.route('/api/appointments', methods=['GET'])
def get_appts(): return jsonify(read_records('appointments.dat', APPT_FMT, APPT_SIZE, parse_appt))

@app.route('/api/appointments', methods=['POST'])
def add_appt():
    data = request.json
    all_a = read_records('appointments.dat', APPT_FMT, APPT_SIZE, parse_appt)
    data['id'] = max([a['id'] for a in all_a] + [0]) + 1
    data['is_active'] = 1
    write_records('appointments.dat', APPT_FMT, APPT_SIZE, serialize_appt, [data])
    return jsonify({'success': True, 'appointment': data})

# -- Billing --
def parse_bill(r):
    return {
        'id': r[0], 'patient_id': r[1], 'room_charges': r[2], 'medicine_charges': r[3],
        'doctor_fee': r[4], 'total': r[5], 'paid_status': r[6], 'bill_date': decode_str(r[7]), 'is_active': r[8]
    }
def serialize_bill(b):
    return (b['id'], b['patient_id'], b['room_charges'], b['medicine_charges'], b['doctor_fee'], b['total'], b['paid_status'], encode_str(b['bill_date'], 12), b.get('is_active', 1))

@app.route('/api/billing', methods=['GET'])
def get_bills(): return jsonify(read_records('billing.dat', BILL_FMT, BILL_SIZE, parse_bill))

@app.route('/api/billing', methods=['POST'])
def add_bill():
    data = request.json
    all_b = read_records('billing.dat', BILL_FMT, BILL_SIZE, parse_bill)
    data['id'] = max([b['id'] for b in all_b] + [0]) + 1
    data['is_active'] = 1
    write_records('billing.dat', BILL_FMT, BILL_SIZE, serialize_bill, [data])
    return jsonify({'success': True, 'bill': data})

# -- Wards --
def parse_ward(r):
    w = {'id': r[0], 'type': decode_str(r[1]), 'total_beds': r[2], 'occupied_beds': r[3], 'is_active': r[-1]}
    w['patients'] = list(r[4:34])
    return w
def serialize_ward(w):
    pats = w.get('patients', [0]*30)
    while len(pats) < 30: pats.append(0)
    return (w['id'], encode_str(w['type'], 30), w['total_beds'], w['occupied_beds'], *pats[:30], w.get('is_active', 1))

@app.route('/api/wards', methods=['GET'])
def get_wards(): return jsonify(read_records('wards.dat', WARD_FMT, WARD_SIZE, parse_ward))

@app.route('/api/wards', methods=['POST'])
def add_ward():
    data = request.json
    all_w = read_records('wards.dat', WARD_FMT, WARD_SIZE, parse_ward)
    data['id'] = max([w['id'] for w in all_w] + [0]) + 1
    data['is_active'] = 1
    if 'patients' not in data: data['patients'] = [0]*30
    write_records('wards.dat', WARD_FMT, WARD_SIZE, serialize_ward, [data])
    return jsonify({'success': True, 'ward': data})

# -- Staff --
def parse_staff(r):
    return {
        'id': r[0], 'name': decode_str(r[1]), 'role': decode_str(r[2]),
        'department': decode_str(r[3]), 'phone': decode_str(r[4]),
        'salary': r[5], 'is_active': r[6]
    }
def serialize_staff(s):
    return (s['id'], encode_str(s['name'], 60), encode_str(s['role'], 40), encode_str(s['department'], 40), encode_str(s['phone'], 20), float(s['salary']), s.get('is_active', 1))

@app.route('/api/staff', methods=['GET'])
def get_staff(): return jsonify(read_records('staff.dat', STAFF_FMT, STAFF_SIZE, parse_staff))

@app.route('/api/staff', methods=['POST'])
def add_staff():
    data = request.json
    all_s = read_records('staff.dat', STAFF_FMT, STAFF_SIZE, parse_staff)
    data['id'] = max([s['id'] for s in all_s] + [0]) + 1
    data['is_active'] = 1
    write_records('staff.dat', STAFF_FMT, STAFF_SIZE, serialize_staff, [data])
    return jsonify({'success': True, 'staff': data})

# -- Serve Web UI --
@app.route('/')
def serve_ui():
    return send_from_directory('web', 'index.html')

@app.route('/<path:path>')
def serve_static(path):
    return send_from_directory('web', path)

if __name__ == '__main__':
    print("Starting HMS Web Backend on http://127.0.0.1:5000")
    # ensure db dir
    if not os.path.exists(DB_DIR):
        os.makedirs(DB_DIR)
    # create files if missing
    for d in ['patients.dat', 'doctors.dat', 'appointments.dat', 'billing.dat', 'wards.dat', 'staff.dat', 'users.dat']:
        p = os.path.join(DB_DIR, d)
        if not os.path.exists(p):
            open(p, 'wb').close()
            
    app.run(debug=True, port=5000)
