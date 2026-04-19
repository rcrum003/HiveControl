#!/usr/bin/env python3
"""
Compute environmental compensation coefficients from calibration data.

Performs a least-squares linear regression on weight vs temperature and
humidity to determine how much the load cell drifts per degree F and
per percent relative humidity.

Usage:
    python3 compute_compensation.py /path/to/hive-data.db

Reads from the calibration_data table and writes coefficients back
to the hiveconfig table.
"""

import sqlite3
import sys


def compute_coefficients(db_path: str) -> None:
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()

    cursor.execute("""
        SELECT net_weight, ambient_temp_f, ambient_humidity
        FROM calibration_data
        WHERE net_weight IS NOT NULL
          AND ambient_temp_f IS NOT NULL
          AND ambient_humidity IS NOT NULL
        ORDER BY date
    """)
    rows = cursor.fetchall()

    if len(rows) < 10:
        print(f"ERROR: Only {len(rows)} valid samples. Need at least 10.")
        conn.close()
        sys.exit(1)

    weights = [r[0] for r in rows]
    temps = [r[1] for r in rows]
    humidities = [r[2] for r in rows]
    n = len(rows)

    mean_w = sum(weights) / n
    mean_t = sum(temps) / n
    mean_h = sum(humidities) / n

    # Multiple linear regression: weight = a + b*temp + c*humidity
    # Using normal equations for 2 predictors
    # [sum(t^2)   sum(t*h)] [b]   [sum(t*w)]
    # [sum(t*h)   sum(h^2)] [c] = [sum(h*w)]
    # where t, h, w are mean-centered

    dt = [t - mean_t for t in temps]
    dh = [h - mean_h for h in humidities]
    dw = [w - mean_w for w in weights]

    sum_tt = sum(a * b for a, b in zip(dt, dt))
    sum_hh = sum(a * b for a, b in zip(dh, dh))
    sum_th = sum(a * b for a, b in zip(dt, dh))
    sum_tw = sum(a * b for a, b in zip(dt, dw))
    sum_hw = sum(a * b for a, b in zip(dh, dw))

    det = sum_tt * sum_hh - sum_th * sum_th
    if abs(det) < 1e-10:
        print("ERROR: Singular matrix — temperature and humidity may be perfectly correlated.")
        print("Need more variation in environmental conditions.")
        conn.close()
        sys.exit(1)

    temp_coeff = (sum_hh * sum_tw - sum_th * sum_hw) / det
    humidity_coeff = (sum_tt * sum_hw - sum_th * sum_tw) / det

    # R-squared to assess fit quality
    ss_res = sum((dw[i] - temp_coeff * dt[i] - humidity_coeff * dh[i]) ** 2 for i in range(n))
    ss_tot = sum(d ** 2 for d in dw)
    r_squared = 1.0 - (ss_res / ss_tot) if ss_tot > 0 else 0.0

    weight_range = max(weights) - min(weights)
    temp_range = max(temps) - min(temps)
    humidity_range = max(humidities) - min(humidities)

    print("============================================")
    print("Compensation Coefficients Computed")
    print("============================================")
    print(f"  Samples:          {n}")
    print(f"  Weight range:     {weight_range:.3f} lbs")
    print(f"  Temp range:       {temp_range:.1f} F")
    print(f"  Humidity range:   {humidity_range:.1f} %RH")
    print(f"")
    print(f"  Temp coefficient:     {temp_coeff:.6f} lbs/F")
    print(f"  Humidity coefficient: {humidity_coeff:.6f} lbs/%RH")
    print(f"  Reference temp:       {mean_t:.1f} F")
    print(f"  Reference humidity:   {mean_h:.1f} %RH")
    print(f"  R-squared:            {r_squared:.4f}")
    print(f"")

    if r_squared < 0.3:
        print("ERROR: R-squared is too low ({:.4f}). Environmental factors do not".format(r_squared))
        print("explain enough of the drift. Possible causes:")
        print("  - Not enough variation in temperature/humidity during calibration")
        print("  - Drift is from other sources (mechanical settling, creep)")
        print("  - Need more calibration data (run for 48-72 hours)")
        print("")
        print("Compensation NOT enabled. Collect more data and try again.")
        conn.close()
        sys.exit(1)

    predicted_correction_temp = abs(temp_coeff * temp_range)
    predicted_correction_hum = abs(humidity_coeff * humidity_range)
    print(f"  Predicted correction over observed temp range:     {predicted_correction_temp:.3f} lbs")
    print(f"  Predicted correction over observed humidity range: {predicted_correction_hum:.3f} lbs")
    print(f"  Total drift explained:                            {predicted_correction_temp + predicted_correction_hum:.3f} lbs")
    print(f"  Total drift observed:                             {weight_range:.3f} lbs")

    # Add columns if they don't exist (safe for re-runs)
    for col, coltype, default in [
        ("WEIGHT_TEMP_COEFF", "REAL", "0"),
        ("WEIGHT_HUMIDITY_COEFF", "REAL", "0"),
        ("WEIGHT_REF_TEMP", "REAL", "NULL"),
        ("WEIGHT_REF_HUMIDITY", "REAL", "NULL"),
        ("WEIGHT_COMPENSATION_ENABLED", "TEXT", "'no'"),
    ]:
        try:
            cursor.execute(f"ALTER TABLE hiveconfig ADD COLUMN {col} {coltype} DEFAULT {default}")
        except sqlite3.OperationalError:
            pass  # Column already exists

    cursor.execute("""
        UPDATE hiveconfig SET
            WEIGHT_TEMP_COEFF = ?,
            WEIGHT_HUMIDITY_COEFF = ?,
            WEIGHT_REF_TEMP = ?,
            WEIGHT_REF_HUMIDITY = ?,
            WEIGHT_COMPENSATION_ENABLED = 'yes'
        WHERE id = 1
    """, (round(temp_coeff, 6), round(humidity_coeff, 6), round(mean_t, 1), round(mean_h, 1)))

    # Increment version so config gets reloaded
    cursor.execute("UPDATE hiveconfig SET version = version + 1 WHERE id = 1")

    conn.commit()

    if cursor.rowcount == 0:
        print("")
        print("ERROR: No row with id=1 in hiveconfig. Coefficients not saved.")
        conn.close()
        sys.exit(1)

    conn.close()

    print("")
    print("Coefficients saved to hiveconfig and compensation ENABLED.")
    print("The weight_monitor.sh script will now apply corrections automatically.")
    print("")
    print("To disable compensation:")
    print("  sqlite3 /home/HiveControl/data/hive-data.db \\")
    print("    \"UPDATE hiveconfig SET WEIGHT_COMPENSATION_ENABLED='no' WHERE id=1;\"")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <path-to-hive-data.db>")
        sys.exit(1)
    compute_coefficients(sys.argv[1])
