# mqtt_dashboard

Flask + MQTT dashboard to manage “shops” and “bands”, visualize online/offline status, and toggle per‑band alerts.

## Stack and layout
- Flask app: `app.py`
- Templates: `templates/index.html`, `templates/shop.html`
- SQLite DB: `bands.db` with tables shops(id, shop_name, start_date, end_date) and bands(id, band_name, shop_id)
- MQTT: paho‑mqtt client thread for subscriptions and control publishing

## Install and run
- Python 3.x
- pip install flask paho-mqtt
- Configure broker in `app.py`:
  - Host/port: mqtt_client.connect(...)
  - TLS CA path: mqtt_client.tls_set("<path-to-ca.crt>")
  - Username/password: mqtt_client.username_pw_set("<user>", "<pass>")
- Initialize DB and start: python app.py → http://localhost:5000

## MQTT topics
- Subscribes: tapband/# and tapband/alerts
- Publishes control: tapband/control/{shop}/{band} with payload "enable" or "disable"
- Device status feed (expected): tapband/{shop}/Band_XXX with JSON payloads like {"uptime": "HH:MM"}
- An offline checker thread marks devices offline if no message for >10s; subscription state updates every 5s

## Pages
- Dashboard “/”: create shops, view subscription window, counts of total/online bands
- Shop “/shop/<id>”: list bands, toggle alert (publishes control topic), add/delete bands

## Notes and tips
- Keep TLS CA path valid for your broker; if you use a public broker without TLS, remove tls_set and change port
- For production: run gunicorn/uwsgi behind a reverse proxy; move credentials to environment variables
- The web UI is intentionally minimal; extend templates for richer views (signal strength, last seen, etc.)
