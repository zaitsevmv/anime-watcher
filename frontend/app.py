from flask import Flask, render_template, request, jsonify, session, redirect, url_for, send_from_directory
import requests 
from dotenv import load_dotenv
from flask_session import Session
import redis
import os
import time
import logging
import json


load_dotenv()

app = Flask(__name__, static_folder='static')
app.config['SECRET_KEY'] = os.getenv('FLASK_SECRET_KEY')
API_BASE_URL = os.getenv('API_BASE_URL')

app.config['SESSION_TYPE'] = 'redis'
app.config['SESSION_REDIS'] = redis.from_url('redis://127.0.0.1:6379')

server_session = Session(app)

logging.basicConfig(filename="debug.log", level=logging.DEBUG, format="%(asctime)s - %(levelname)s - %(message)s")


def check_file(path):
    return os.path.exists(app.static_folder + path)


def api_request(method, endpoint, data=None, headers=None):
    """Helper function for making API calls"""
    url = f"{API_BASE_URL}/{endpoint}"
    try:
        response = requests.request(method, url, json=data, headers=headers)
        return response.json()
    except Exception as e:
        logging.debug(f"API Error: {e}")
        return None


@app.route('/')
def index():
    if 'user_id' not in session:
        return redirect(url_for('login'))

    user_id = session['user_id']
    fav_response = api_request('GET', f'users/favourites?user_id={user_id}')
    favorite_anime_ids = fav_response.get('anime_ids', '[]') if fav_response and fav_response.get('success') else '[]'
    
    last_video_response = api_request('GET', f'users/last_video?user_id={user_id}')
    last_video = last_video_response.get('last_video', '0') if last_video_response and last_video_response.get('success') else '0'

    return render_template('index.html', anime_ids=json.loads(favorite_anime_ids), last_watched=last_video)


@app.route('/images/anime/<filename>')
def anime_image(filename):
    images_dir = os.path.join(app.static_folder, 'images', 'anime')
    return send_from_directory(images_dir, filename)


@app.route('/videos/<filename>')
def anime_video(filename):
    videos_dir = os.path.join(app.static_folder, 'videos')
    return send_from_directory(videos_dir, filename)


@app.route('/anime_by_video/<video_id>')
def get_anime_by_video(video_id):
    video_path = f'/videos/{video_id}.mp4' 
    logging.debug(check_file(video_path))
    if check_file(video_path):
        anime_id, sep, episode = video_id.rpartition('_')
        if sep:
            response = api_request('GET', f'anime/details?anime_id={anime_id}')
            if response and response.get('success'):
                anime_data = response.get('anime', '')
            else:
                return {}
            anime_data = json.loads(anime_data)
            return {'title': anime_data['title'], 'episode': episode}
    return {}


@app.route('/anime/<anime_id>')
def anime_page(anime_id):
    if not anime_id:
        return
    response = api_request('GET', f'anime/details?anime_id={anime_id}')
    if response and response.get('success'):
        anime_data = response.get('anime', '')
    else: 
        return
    if anime_data == '':
        return
    
    anime_data = json.loads(anime_data)
    try:
        oid_obj = anime_data['_id']
        anime_data['id'] = oid_obj['$oid']
    except (json.JSONDecodeError, KeyError, TypeError):
        anime_data['id'] = anime_data.get('_id')
        
    image_path = f'/images/anime/{anime_data["id"]}.jpg'
    default_path = "/images/anime/default.jpg"

    anime_data['image_url'] = image_path if check_file(image_path) else default_path    
    return render_template("anime.html", anime=anime_data)


@app.route('/add_favourite', methods=['POST'])
def add_favourite():
    logging.debug(request.json)
    anime_id = request.json
    response = api_request('POST', 'anime/favourites/add', {
        'user_id': session['user_id'],
        'anime_id': anime_id
    })
    if response and response.get('success'):
        return jsonify({"success": True})
    return jsonify({"success": False})


@app.route('/search')
def anime():
    return render_template('search.html')


@app.route('/search_anime')
def search_anime():
    query = request.args.get('q', '').strip()

    if not query:
        return jsonify([])
    response = api_request('GET', f'anime/search?query={query}')
    
    if response and response.get('success'):
        return jsonify(response.get('anime_ids', []))  # Return list of found anime

    return jsonify([])  # Return empty on error


@app.route('/anime_details/<anime_id>')
def anime_details(anime_id):
    response = api_request('GET', f'anime/details?anime_id={anime_id}')
    if response and response.get('success'):
        data = json.loads(response.get('anime', ''))
        image_path = f'/images/anime/{data["_id"]["$oid"]}.jpg'
        default_path = "/images/anime/default.jpg"

        data['image_url'] = image_path if check_file(image_path) else default_path
        return data
    return {}


@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        # API call to backend for authentication
        response = api_request('POST', 'auth/login', {
            'login': request.form['login'],
            'password_hash': request.form['password']
        })
        
        if response and response.get('success'):
            session['user_id'] = response['user_id']
            return redirect(url_for('index'))
    return render_template('login.html')


@app.route('/register', methods=['GET', 'POST'])
def register():
    if request.method == 'POST':
        # API call to backend for user registration
        response = api_request('POST', 'auth/register', {
            'login': request.form['login'],
            'email': request.form['email'],
            'password_hash': request.form['password']
        })
        
        if response and response.get('success'):
            session['user_id'] = response['user_id']
            return redirect(url_for('index'))
    return render_template('register.html')


@app.route('/logout')
def logout():
    session.pop('user_id', None)
    return redirect(url_for('index'))


@app.route('/chat')
def chat():
    if 'user_id' not in session:
        return redirect(url_for('login'))
    return render_template('chat.html')


def get_username(user_id):
    redis_conn = redis.from_url('redis://127.0.0.1:6379')
    username = redis_conn.get(f"user:{user_id}:name")
    if username:
        return username.decode('utf-8')

    response = api_request('GET', f'users/name?user_id={user_id}')
    if response and response.get('success'):
        user_data = response.get('user')
        if isinstance(user_data, str):
            user_data = json.loads(user_data)
        user_name = user_data.get('name')
        
        logging.debug(user_data)

        redis_conn.set(f"user:{user_id}:name", user_name)
        return user_name

    return 'Anonim' 


@app.route('/get_messages')
def get_messages():
    """Fetch messages after a given timestamp from the backend."""
    last_timestamp = request.args.get('after', 0)  # Default to 0 if no timestamp is provided
    response = api_request('GET', f'chat/messages?after={last_timestamp}')

    if response and response.get('success'):
        messages_list = response.get('messages', [])
        updated_message_list = []
        for msg in messages_list:
            if isinstance(msg, str):
                msg = json.loads(msg)
                
            user_name = get_username(msg['user_id'])
            msg['user_name'] = str(user_name)
            updated_message_list.append(msg)
        return jsonify(updated_message_list)
    return jsonify([])  


@app.route('/send_message', methods=['POST'])
def send_message():
    if 'user_id' not in session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 403

    message_text = request.json.get('message')
    if not message_text:
        return jsonify({'success': False, 'error': 'Empty message'}), 400

    response = api_request('POST', 'chat/send', data={
        'user_id': session['user_id'],
        'message': message_text,
        'timestamp_ms': int(time.time() * 1000)  # Add milliseconds timestamp
    })

    if response and response.get('success'):
        return jsonify({'success': True})
    return jsonify({'success': False, 'error': 'Failed to send message'}), 500