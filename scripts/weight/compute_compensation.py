#!/usr/bin/env python3
"""
Compute environmental compensation coefficients for load cell drift.

Two modes:
  Manual:  python3 compute_compensation.py <db_path>
           Reads from calibration_data table (static weight calibration).

  Auto:    python3 compute_compensation.py --auto <db_path>
           Reads nighttime (midnight-5am) data from weight_readings table.
           Does not change WEIGHT_COMPENSATION_ENABLED.
"""

import sqlite3
import sys
from datetime import datetime


def ensure_columns(cursor: sqlite3.Cursor) -> None:
    for col, coltype, default in [
        ("WEIGHT_TEMP_COEFF", "REAL", "0"),
        ("WEIGHT_HUMIDITY_COEFF", "REAL", "0"),
        ("WEIGHT_REF_TEMP", "REAL", "NULL"),
        ("WEIGHT_REF_HUMIDITY", "REAL", "NULL"),
        ("WEIGHT_COMPENSATION_ENABLED", "TEXT", "'no'"),
        ("WEIGHT_MONITOR_INTERVAL", "INTEGER", "15"),
        ("WEIGHT_LAST_CALIBRATED", "TEXT", "NULL"),
        ("WEIGHT_CALIBRATION_R2", "REAL", "NULL"),
    ]:
        try:
            cursor.execute(
                f"ALTER TABLE hiveconfig ADD COLUMN {col} {coltype} DEFAULT {default}"
            )
        except sqlite3.OperationalError:
            pass


def compute_regression(
    weights: list[float], temps: list[float], humidities: list[float]
) -> tuple[float, float, float, float, float]:
    n = len(weights)
    mean_w = sum(weights) / n
    mean_t = sum(temps) / n
    mean_h = sum(humidities) / n

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
        raise ValueError("Singular matrix — need more variation in conditions")

    temp_coeff = (sum_hh * sum_tw - sum_th * sum_hw) / det
    humidity_coeff = (sum_tt * sum_hw - sum_th * sum_tw) / det

    ss_res = sum(
        (dw[i] - temp_coeff * dt[i] - humidity_coeff * dh[i]) ** 2 for i in range(n)
    )
    ss_tot = sum(d ** 2 for d in dw)
    r_squared = 1.0 - (ss_res / ss_tot) if ss_tot > 0 else 0.0

    return temp_coeff, humidity_coeff, mean_t, mean_h, r_squared


def compute_auto(db_path: str) -> None:
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    ensure_columns(cursor)

    cursor.execute("""
        SELECT net_weight, ambient_temp_f, ambient_humidity
        FROM weight_readings
        WHERE date >= datetime('now', '-14 days')
          AND CAST(strftime('%H', date) AS INTEGER) BETWEEN 0 AND 4
          AND net_weight IS NOT NULL
          AND ambient_temp_f IS NOT NULL AND ambient_temp_f != 'null'
          AND ambient_humidity IS NOT NULL AND ambient_humidity != 'null'
        ORDER BY date
    """)
    rows = cursor.fetchall()

    if len(rows) < 50:
        print(f"AUTO-CAL: Only {len(rows)} nighttime samples. Need 50+. Skipping.")
        conn.close()
        return

    weights = [r[0] for r in rows]
    temps = [r[1] for r in rows]
    humidities = [r[2] for r in rows]

    temp_range = max(temps) - min(temps)
    if temp_range < 5.0:
        print(f"AUTO-CAL: Temp range only {temp_range:.1f}F. Need 5+ degree spread. Skipping.")
        conn.close()
        return

    try:
        temp_coeff, humidity_coeff, ref_temp, ref_humidity, r_squared = compute_regression(
            weights, temps, humidities
        )
    except ValueError as e:
        print(f"AUTO-CAL: {e}. Skipping.")
        conn.close()
        return

    if r_squared < 0.15:
        print(f"AUTO-CAL: R²={r_squared:.4f} too low. Environmental drift minimal. Skipping.")
        conn.close()
        return

    now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    cursor.execute("""
        UPDATE hiveconfig SET
            WEIGHT_TEMP_COEFF = ?,
            WEIGHT_HUMIDITY_COEFF = ?,
            WEIGHT_REF_TEMP = ?,
            WEIGHT_REF_HUMIDITY = ?,
            WEIGHT_LAST_CALIBRATED = ?,
            WEIGHT_CALIBRATION_R2 = ?,
            version = version + 1
        WHERE id = 1
    """, (
        round(temp_coeff, 6),
        round(humidity_coeff, 6),
        round(ref_temp, 1),
        round(ref_humidity, 1),
        now,
        round(r_squared, 4),
    ))

    conn.commit()
    conn.close()

    print(f"AUTO-CAL: Success — {len(rows)} samples, R²={r_squared:.4f}")
    print(f"  Temp coeff: {temp_coeff:.6f} lbs/F")
    print(f"  Humidity coeff: {humidity_coeff:.6f} lbs/%RH")
    print(f"  Ref temp: {ref_temp:.1f}F, Ref humidity: {ref_humidity:.1f}%RH")


def compute_manual(db_path: str) -> None:
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    ensure_columns(cursor)

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

    try:
        temp_coeff, humidity_coeff, ref_temp, ref_humidity, r_squared = compute_regression(
            weights, temps, humidities
        )
    except ValueError as e:
        print(f"ERROR: {e}")
        conn.close()
        sys.exit(1)

    n = len(rows)
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
    print(f"  Reference temp:       {ref_temp:.1f} F")
    print(f"  Reference humidity:   {ref_humidity:.1f} %RH")
    print(f"  R-squared:            {r_squared:.4f}")

    if r_squared < 0.3:
        print(f"\nERROR: R-squared too low. Compensation NOT enabled.")
        conn.close()
        sys.exit(1)

    now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    cursor.execute("""
        UPDATE hiveconfig SET
            WEIGHT_TEMP_COEFF = ?,
            WEIGHT_HUMIDITY_COEFF = ?,
            WEIGHT_REF_TEMP = ?,
            WEIGHT_REF_HUMIDITY = ?,
            WEIGHT_COMPENSATION_ENABLED = 'yes',
            WEIGHT_LAST_CALIBRATED = ?,
            WEIGHT_CALIBRATION_R2 = ?,
            version = version + 1
        WHERE id = 1
    """, (
        round(temp_coeff, 6),
        round(humidity_coeff, 6),
        round(ref_temp, 1),
        round(ref_humidity, 1),
        now,
        round(r_squared, 4),
    ))

    conn.commit()
    conn.close()
    print("\nCoefficients saved and compensation ENABLED.")


if __name__ == "__main__":
    if len(sys.argv) == 3 and sys.argv[1] == "--auto":
        compute_auto(sys.argv[2])
    elif len(sys.argv) == 2:
        compute_manual(sys.argv[1])
    else:
        print(f"Usage: {sys.argv[0]} [--auto] <path-to-hive-data.db>")
        sys.exit(1)
