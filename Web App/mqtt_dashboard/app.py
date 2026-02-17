from flask import Flask, render_template, request, redirect, url_for, jsonify
import sqlite3
import time
import threading
import json
import paho.mqtt.client as mqtt
import os
from datetime import datetime

app = Flask(__name__)
DATABASE = "bands.db"

# MQTT Setup
alert_topic = "tapband/alerts"
control_topic_prefix = "tapband/control/"
mqtt_client = mqtt.Client()

devices = {}  # {band: {...}}
last_seen = {}

# ---------- DATABASE ----------
def init_db():
    with sqlite3.connect(DATABASE) as conn:
        conn.execute('''
            CREATE TABLE IF NOT EXISTS shops (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                shop_name TEXT UNIQUE,
                start_date TEXT,
                end_date TEXT
            )
        ''')
        conn.execute('''
            CREATE TABLE IF NOT EXISTS bands (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                band_name TEXT UNIQUE,
                shop_id INTEGER,
                FOREIGN KEY(shop_id) REFERENCES shops(id)
            )
        ''')

def get_shops():
    with sqlite3.connect(DATABASE) as conn:
        return conn.execute("SELECT id, shop_name, start_date, end_date FROM shops").fetchall()

def get_shop(shop_id):
    with sqlite3.connect(DATABASE) as conn:
        return conn.execute("SELECT id, shop_name, start_date, end_date FROM shops WHERE id = ?", (shop_id,)).fetchone()

def get_bands_for_shop(shop_id):
    with sqlite3.connect(DATABASE) as conn:
        return conn.execute("SELECT band_name FROM bands WHERE shop_id = ?", (shop_id,)).fetchall()

def add_shop(start_date, end_date):
    with sqlite3.connect(DATABASE) as conn:
        existing = conn.execute("SELECT shop_name FROM shops").fetchall()
        names = {name for (name,) in existing}
        i = 1
        while True:
            name = f"Shop_{i:03d}"
            if name not in names:
                break
            i += 1
        conn.execute("INSERT INTO shops (shop_name, start_date, end_date) VALUES (?, ?, ?)", (name, start_date, end_date))
        conn.commit()

def add_band(shop_id):
    with sqlite3.connect(DATABASE) as conn:
        existing = conn.execute("SELECT band_name FROM bands").fetchall()
        names = {name for (name,) in existing}
        i = 1
        while True:
            band = f"Band_{i:03d}"
            if band not in names:
                break
            i += 1
        conn.execute("INSERT INTO bands (band_name, shop_id) VALUES (?, ?)", (band, shop_id))
        conn.commit()
    return band

def delete_band(band_name):
    with sqlite3.connect(DATABASE) as conn:
        conn.execute("DELETE FROM bands WHERE band_name = ?", (band_name,))
    devices.pop(band_name, None)
    last_seen.pop(band_name, None)

# ---------- MQTT ----------
def on_connect(client, userdata, flags, rc):
    client.subscribe("tapband/#")
    client.subscribe(alert_topic)

def on_message(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload.decode()

    parts = topic.split('/')
    if len(parts) == 3 and parts[0] == 'tapband' and parts[2].startswith("Band_"):
        shop = parts[1]
        band = parts[2]
        if band in devices:
            devices[band]["status"] = "online"
            try:
                data = json.loads(payload)
                uptime = data.get("uptime", "00:00")
                devices[band]["uptime"] = uptime
                devices[band]["last_uptime"] = uptime
            except:
                devices[band]["uptime"] = "00:00"
            last_seen[band] = time.time()

    elif topic.startswith(control_topic_prefix):
        parts = topic.split('/')
        if len(parts) == 4:
            band = parts[3]
            if band in devices:
                devices[band]["alert_enabled"] = (payload == "enable")

    elif topic == alert_topic:
        print(f"Alert received: {payload}")

mqtt_client.username_pw_set("zaeem7744", "zaeem7744")
mqtt_client.tls_set("C:/emqxsl-ca.crt")
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

# Try to connect to MQTT broker, but don't crash the web app if it fails
try:
    mqtt_client.connect("g8807071.ala.eu-central-1.emqxsl.com", 8883)
    mqtt_client.loop_start()
    print("[MQTT] Connected to broker g8807071.ala.eu-central-1.emqxsl.com:8883")
except Exception as e:
    print(f"[MQTT] Connection failed: {e}. Dashboard will still run without MQTT.")

# ---------- THREAD: Offline Devices & Subscription Check ----------
def offline_checker():
    while True:
        now = time.time()
        current_date = datetime.now().date()

        with sqlite3.connect(DATABASE) as conn:
            bands = conn.execute("""
                SELECT b.band_name, s.end_date
                FROM bands b
                JOIN shops s ON b.shop_id = s.id
            """).fetchall()

        for band_name, end_date in bands:
            expired = datetime.strptime(end_date, "%Y-%m-%d").date() < current_date
            if band_name not in devices:
                devices[band_name] = {"status": "offline", "uptime": "00:00", "last_uptime": None, "offline_since": None, "alert_enabled": not expired}
            if expired:
                devices[band_name]["alert_enabled"] = False

            last = last_seen.get(band_name, 0)
            if now - last > 10:
                if devices[band_name]["status"] == "online":
                    devices[band_name]["offline_since"] = time.strftime("%Y-%m-%d %H:%M:%S")
                devices[band_name]["status"] = "offline"
            else:
                devices[band_name]["offline_since"] = None

        time.sleep(5)

# ---------- ROUTES ----------
@app.route("/")
def index():
    shops = []
    raw_shops = get_shops()
    for s in raw_shops:
        total_bands, online_bands = get_bands_count(s[0])
        shops.append({
            "id": s[0],
            "name": s[1],
            "start_date": s[2],
            "end_date": s[3],
            "total_bands": total_bands,
            "online_bands": online_bands
        })
    return render_template("index.html", shops=shops)

def get_bands_count(shop_id):
    with sqlite3.connect(DATABASE) as conn:
        total = conn.execute("SELECT COUNT(*) FROM bands WHERE shop_id = ?", (shop_id,)).fetchone()[0]
        online = 0
        bands = conn.execute("SELECT band_name FROM bands WHERE shop_id = ?", (shop_id,)).fetchall()
        for (band_name,) in bands:
            if devices.get(band_name, {}).get("status") == "online":
                online += 1
        return total, online

@app.route("/add_shop", methods=["POST"])
def create_shop():
    start = request.form["start_date"]
    end = request.form["end_date"]
    add_shop(start, end)
    return redirect("/")

@app.route("/shop/<int:shop_id>")
def shop_page(shop_id):
    shop = get_shop(shop_id)
    if not shop:
        return "Shop not found", 404
    bands = get_bands_for_shop(shop_id)
    band_data = {band: devices.get(band, {"status": "offline", "uptime": "00:00", "last_uptime": None, "offline_since": None, "alert_enabled": False}) for (band,) in bands}
    return render_template("shop.html", shop=shop, devices=band_data)

@app.route("/shop/<int:shop_id>/add_band", methods=["POST"])
def add_band_to_shop(shop_id):
    band_name = add_band(shop_id)
    devices[band_name] = {"status": "offline", "uptime": "00:00", "last_uptime": None, "offline_since": None, "alert_enabled": True}
    return redirect(url_for("shop_page", shop_id=shop_id))


@app.route("/toggle_alert/<shop>/<band>", methods=["POST"])
def toggle_alert(shop, band):
    if band not in devices:
        return jsonify({"error": "Invalid band"}), 404

    new_status = not devices[band]["alert_enabled"]
    devices[band]["alert_enabled"] = new_status
    msg = "enable" if new_status else "disable"

    # FIXED: now includes the shop name in the topic
    mqtt_client.publish(f"{control_topic_prefix}{shop}/{band}", msg)

    return jsonify({"alert_enabled": new_status})


@app.route("/delete_band/<band>", methods=["POST"])
def delete_band_route(band):
    delete_band(band)
    return redirect(request.referrer)

@app.route("/delete_shop/<int:shop_id>", methods=["POST"])
def delete_shop(shop_id):
    with sqlite3.connect(DATABASE) as conn:
        bands = conn.execute("SELECT band_name FROM bands WHERE shop_id = ?", (shop_id,)).fetchall()
        for (band_name,) in bands:
            devices.pop(band_name, None)
            last_seen.pop(band_name, None)

        conn.execute("DELETE FROM bands WHERE shop_id = ?", (shop_id,))
        conn.execute("DELETE FROM shops WHERE id = ?", (shop_id,))
        conn.commit()
    return redirect("/")


# ---------- START ----------
init_db()
threading.Thread(target=offline_checker, daemon=True).start()

if __name__ == "__main__":
    # Listen on all interfaces so you can also access from phone/other devices
    app.run(host="0.0.0.0", port=5050, debug=True)
