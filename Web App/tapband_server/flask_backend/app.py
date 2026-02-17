from flask import Flask, render_template, request, redirect, url_for, flash
from flask_sqlalchemy import SQLAlchemy
from flask_socketio import SocketIO
from datetime import datetime, date
import paho.mqtt.client as mqtt
from threading import Thread
from flask_migrate import Migrate
import os

# Initialize Flask
app = Flask(__name__)
basedir = os.path.abspath(os.path.dirname(__file__))
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///' + os.path.join(basedir, 'instance', 'tapband.db')
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
app.config['SECRET_KEY'] = 'your-secret-key-here'
db = SQLAlchemy(app)
socketio = SocketIO(app)
migrate = Migrate(app, db)

# MQTT Configuration
MQTT_BROKER = "192.168.0.102"  # Replace with your broker address
MQTT_USER = "admin"
MQTT_PASS = "zaeem7744"
MQTT_TOPIC_STATUS = "shop/+/band/+/status"

# Database Models
class Shop(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(100), unique=True, nullable=False)
    subscription_start = db.Column(db.Date, nullable=False)
    subscription_end = db.Column(db.Date, nullable=False)
    is_active = db.Column(db.Boolean, default=True)
    bands = db.relationship('Band', backref='shop', lazy=True)

    def is_subscription_active(self):
        return self.is_active and (date.today() <= self.subscription_end)

class Band(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    band_id = db.Column(db.String(50), unique=True, nullable=False)
    shop_id = db.Column(db.Integer, db.ForeignKey('shop.id'), nullable=False)
    is_active = db.Column(db.Boolean, default=True)
    last_seen = db.Column(db.DateTime)
    status = db.Column(db.String(20), default='offline')
    battery_level = db.Column(db.Integer)  # Ensure this line is present
    wifi_ssid = db.Column(db.String(100))
    signal_strength = db.Column(db.Integer)
    uptime = db.Column(db.Integer, default=0)

# MQTT Client Functions

def on_mqtt_message(client, userdata, message):
    payload = message.payload.decode()
    topic_parts = message.topic.split('/')
    
    print(f"Received message on topic: {message.topic} with payload: {payload}")
    
    if len(topic_parts) == 5 and topic_parts[2] == 'band' and topic_parts[4] == 'status':
        shop_id = int(topic_parts[1])
        band_id = topic_parts[3]
        
        try:
            data = json.loads(payload)
            status = data.get("status")
            timestamp = data.get("timestamp")
            
            print(f"Updating {band_id} status to {status}")
            update_band_status(band_id, shop_id, [status])
        except Exception as e:
            print(f"Error processing message: {e}")


def on_mqtt_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker with result code " + str(rc))
    client.subscribe("shop/#")  # Subscribe to all topics under "shop/"

def update_band_status(band_id, shop_id, status_data):
    with app.app_context():
        band = Band.query.filter_by(band_id=band_id, shop_id=shop_id).first()
        if band:
            band.status = status_data[0]  # Update status
            band.last_seen = datetime.now()  # Update last seen timestamp
            
            db.session.commit()  # Commit the changes
            print(f"{band_id} status updated to {band.status} at {band.last_seen}")



def start_mqtt_client():
    client = mqtt.Client()
    client.username_pw_set(MQTT_USER, MQTT_PASS)
    client.on_connect = on_mqtt_connect
    client.on_message = on_mqtt_message
    client.connect(MQTT_BROKER)
    client.loop_forever()

# Application Routes
@app.route('/')
def index():
    stats = {
        'total_shops': Shop.query.count(),
        'total_bands': Band.query.count(),
        'active_shops': Shop.query.filter(Shop.subscription_end >= date.today()).count()
    }
    shops = Shop.query.all()
    return render_template('index.html', stats=stats, shops=shops)

@app.route('/shops')
def shop_list():
    shops = Shop.query.all()
    return render_template('shops.html', shops=shops)

@app.route('/shop/<int:shop_id>')
def shop_detail(shop_id):
    shop = Shop.query.get_or_404(shop_id)
    return render_template('shop_detail.html', shop=shop)

@app.route('/bands')
def band_list():
    shop_id = request.args.get('shop_id')
    query = Band.query.join(Shop).order_by(Band.band_id)
    
    if shop_id:
        shop = Shop.query.get_or_404(shop_id)
        query = query.filter(Band.shop_id == shop_id)
        bands = query.all()
        return render_template('bands.html', 
                            bands=bands, 
                            shop=shop, 
                            shops=None,
                            now=datetime.now(),
                            date=date.today())
    else:
        bands = query.all()
        shops = Shop.query.order_by(Shop.name).all()
        return render_template('bands.html', 
                            bands=bands, 
                            shop=None, 
                            shops=shops,
                            now=datetime.now(),
                            date=date.today())

@app.route('/band/<band_id>')
def band_detail(band_id):
    band = Band.query.filter_by(band_id=band_id).first_or_404()
    return render_template('band_detail.html', band=band)

@app.route('/add_shop', methods=['GET', 'POST'])
def add_shop():
    if request.method == 'POST':
        name = request.form['name']
        start_date = datetime.strptime(request.form['start_date'], '%Y-%m-%d').date()
        end_date = datetime.strptime(request.form['end_date'], '%Y-%m-%d').date()
        
        shop = Shop(name=name, subscription_start=start_date, subscription_end=end_date)
        db.session.add(shop)
        db.session.commit()
        flash('Shop added successfully!', 'success')
        return redirect(url_for('shop_list'))
    return render_template('add_shop.html')

@app.route('/add_band', methods=['GET', 'POST'])
def add_band():
    if request.method == 'POST':
        band_id = request.form['band_id']
        shop_id = request.form['shop_id']
        
        band = Band(band_id=band_id, shop_id=shop_id)
        db.session.add(band)
        db.session.commit()
        flash('Band added successfully!', 'success')
        return redirect(url_for('band_list'))
    
    shops = Shop.query.all()
    return render_template('add_band.html', shops=shops)

@app.route('/toggle_shop/<int:shop_id>')
def toggle_shop(shop_id):
    shop = Shop.query.get_or_404(shop_id)
    shop.is_active = not shop.is_active
    db.session.commit()
    flash(f'Shop {"activated" if shop.is_active else "deactivated"}!', 'success')
    return redirect(url_for('shop_list'))

@app.route('/toggle_band/<band_id>')
def toggle_band(band_id):
    band = Band.query.filter_by(band_id=band_id).first_or_404()
    band.is_active = not band.is_active
    db.session.commit()
    flash(f'Band {"activated" if band.is_active else "deactivated"}!', 'success')
    return redirect(url_for('band_list'))

def initialize_database():
    """Initialize database with empty tables"""
    with app.app_context():
        os.makedirs(os.path.join(basedir, 'instance'), exist_ok=True)
        db.drop_all()
        db.create_all()
        db.session.commit()

@app.route('/delete_shop/<int:shop_id>', methods=['POST'])
def delete_shop(shop_id):
    shop = Shop.query.get_or_404(shop_id)
    Band.query.filter_by(shop_id=shop_id).delete()
    db.session.delete(shop)
    db.session.commit()
    flash(f'Shop {shop.name} and all its bands have been deleted', 'success')
    return redirect(url_for('shop_list'))

@app.route('/delete_band/<band_id>', methods=['POST'])
def delete_band(band_id):
    band = Band.query.filter_by(band_id=band_id).first_or_404()
    db.session.delete(band)
    db.session.commit()
    flash(f'Band {band_id} has been deleted', 'success')
    return redirect(request.referrer or url_for('band_list'))

if __name__ == '__main__':
    if not os.path.exists(os.path.join(basedir, 'instance', 'tapband.db')):
        initialize_database()

    mqtt_thread = Thread(target=start_mqtt_client)
    mqtt_thread.daemon = True
    mqtt_thread.start()

    socketio.run(app, debug=True)
