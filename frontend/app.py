from flask import Flask, render_template, request, jsonify, session, redirect, url_for, send_from_directory
import requests 
import shutil
from functools import wraps
from dotenv import load_dotenv
from flask_session import Session
from werkzeug.utils import secure_filename
from prometheus_flask_exporter import PrometheusMetrics
from prometheus_client import Counter
import redis
import bcrypt
import os
import time
import logging
import json
import math


load_dotenv()

app = Flask(__name__, static_folder='static')
app.config['SECRET_KEY'] = os.getenv('FLASK_SECRET_KEY')
API_BASE_URL = 'http://192.168.0.10:8080'

app.config['SESSION_TYPE'] = 'redis'
app.config['SESSION_REDIS'] = redis.from_url('redis://192.168.0.10:6379')

server_session = Session(app)

logging.basicConfig(filename="debug.log", level=logging.DEBUG, format="%(asctime)s - %(levelname)s - %(message)s")

metrics = PrometheusMetrics(app)

new_videos_counter = Counter(
    'new_videos_counter', 'Количество загруженных видео'
)

new_images_counter = Counter(
    'new_images_counter', 'Количество отправленных сообщений'
)

new_users_counter = Counter(
    'new_users_counter', 'Количество новых пользователей'
)

banned_users_counter = Counter(
    'banned_users_counter', 'Количество заблокиррованных пользователей'
)

new_anime_counter = Counter(
    'new_anime_counter', 'Количество добавленных аниме'
)

new_messages_counter = Counter(
    'new_messages_counter', 'Количество отправленных сообщений'
)

deleted_messages_counter = Counter(
    'deleted_messages_counter', 'Количество удаленных сообщений'
)

def hash_password(password):
    salt = bcrypt.gensalt()
    hashed_password = bcrypt.hashpw(password.encode('utf-8'), salt)
    return hashed_password.decode('utf-8')


def login_required(f):
    @wraps(f)
    def decorated_function(*args, **kwargs):
        if 'user_id' not in session:  # Or your custom session check
            return redirect(url_for('login'))
        return f(*args, **kwargs)
    return decorated_function


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
@login_required
def index():
    user_id = session['user_id']
    fav_response = api_request('GET', f'users/favourites?user_id={user_id}')
    favorite_anime_ids = fav_response.get('anime_ids', '[]') if fav_response and fav_response.get('success') else '[]'
    
    last_video_response = api_request('GET', f'users/last_video?user_id={user_id}')
    last_video_id = last_video_response.get('last_video_id', '0') if last_video_response and last_video_response.get('success') else '0'
    last_video_ts = last_video_response.get('last_video_timestamp_s', 0) if last_video_response and last_video_response.get('success') else 0

    return render_template('index.html', anime_ids=json.loads(favorite_anime_ids), last_watched=last_video_id, timestamp_s=last_video_ts, user_name=get_username(user_id), user_id=user_id, user_status=get_user_status(user_id))


@app.route('/update_name', methods=['POST'])
def update_name():
    if 'user_id' not in session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 403
    
    data = request.get_json()
    response = api_request('POST', 'users/name/set', data={
        'user_id': data.get('user_id', ''),
        'name': data.get('name', '')
    })
    if response and response.get('success'):
        user_name = data.get('name', '')
        user_id = data.get('user_id', '')
        redis_conn = redis.from_url('redis://192.168.0.10:6379')
        redis_conn.set(f"user:{user_id}:name", user_name)
        return jsonify({"success": True})
    return jsonify({"success": False})


@app.route('/update_last_video', methods=['POST'])
def update_last_video():
    if 'user_id' not in session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 403
    
    data = request.get_json()
    response = api_request('POST', 'users/last_video/set', data={
        'user_id': data.get('user_id', ''),
        'video_id': data.get('video_id', ''),
        'timestamp_s': math.floor(data.get('timestamp_s', 0))
    })
    if response and response.get('success'):
        return jsonify({"success": True})
    return jsonify({"success": False})


@app.route('/images/anime/<filename>', methods=['GET'])
def anime_image(filename):
    images_dir = os.path.join(app.static_folder, 'images', 'anime')
    return send_from_directory(images_dir, filename)


ALLOWED_EXTENSIONS = {'jpg', 'jpeg'}

def allowed_file_image(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS
           

@app.route('/upload_image', methods=['POST'])
def upload_anime_image():
    if 'user_id' not in session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 403
    
    if get_user_status(session['user_id']) <= 2:
        return jsonify({'success': False, 'error': 'Insufficient privileges'}), 403
    
    if 'image' not in request.files:
        return jsonify({'success': False, 'error': 'No file uploaded'})
    
    file = request.files['image']
    if file.filename == '':
        return jsonify({'success': False, 'error': 'No selected file'})
    
    if file and allowed_file_image(file.filename):
        filename = secure_filename(file.filename)
        if filename.rsplit('.', 1)[1].lower() == 'jpeg':
            filename = f"{filename.rsplit('.', 1)[0]}.jpg"

        filepath = os.path.join(app.static_folder, 'images', 'anime', 'uploads', filename)
        file.save(filepath)
        
        new_images_counter.inc()

        return jsonify({
            'success': True,
            'imageUrl': url_for('static', filename=f'images/anime/uploads/{filename}')
        })
    
    return jsonify({'success': False, 'error': 'Only JPG/JPEG files are allowed'})


ALLOWED_EXTENSIONS_VIDEO = {'mp4'}

def allowed_file_video(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS_VIDEO


@app.route('/upload_video', methods=['POST'])
def upload_anime_video():
    if 'user_id' not in session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 403
    
    if get_user_status(session['user_id']) <= 2:
        return jsonify({'success': False, 'error': 'Insufficient privileges'}), 403
    
    if 'video' not in request.files:
        return jsonify({'success': False, 'error': 'No file uploaded'})
    
    file = request.files['video']
    anime_id = request.form.get('anime_id', '')
    episode = request.form.get('episode', '')
    if anime_id == '' or episode == '':
        return jsonify({'success': False, 'error': 'No anime_id or episode'})
    if file.filename == '':
        return jsonify({'success': False, 'error': 'No selected file'})
    
    if file and allowed_file_video(file.filename):
        video_id = f'{anime_id}_{episode}'
        filename = f'{video_id}.mp4'

        filepath = os.path.join(app.static_folder, 'videos', filename)
        file.save(filepath)
        
        response = api_request('POST', 'anime/video/add', {
            'anime_id': anime_id,
            'video_id': episode
        })
        if response and response.get('success'):
            new_videos_counter.inc()
            return jsonify({"success": True})
        
        return jsonify({'success': False})
    
    return jsonify({'success': False, 'error': 'Only MP4 files are allowed'})


@app.route('/delete_video', methods=['POST'])
def delete_anime_video():
    if 'user_id' not in session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 403
    
    if get_user_status(session['user_id']) <= 2:
        return jsonify({'success': False, 'error': 'Insufficient privileges'}), 403

    anime_id = request.json.get('anime_id', '')
    episode = request.json.get('video_id', '')
    if anime_id == '' or episode == '':
        return jsonify({'success': False, 'error': 'No anime_id or episode'})
    
    video_id = f'{anime_id}_{episode}'
    
    filename = f'{video_id}.mp4'
    video_path = f'/videos/{video_id}.mp4' 
    logging.debug(video_path)
    if check_file(video_path):
        video_path = os.path.join(app.static_folder,'videos', filename)
        dest = os.path.join(app.static_folder, 'videos', 'to_delete_' + filename)
        os.rename(video_path, dest)
    
    response = api_request('POST', 'anime/video/remove', {
        'anime_id': anime_id,
        'video_id': episode
    })
    if response and response.get('success'):
        return jsonify({"success": True})
    
    return jsonify({'success': False})


@app.route('/videos/<filename>', methods=['GET'])
def anime_video(filename):
    videos_dir = os.path.join(app.static_folder, 'videos')
    return send_from_directory(videos_dir, filename)


@app.route('/anime_by_video/<video_id>', methods=['GET'])
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
            return {'title': anime_data['title'], 'episode': episode, 'anime_id': anime_id}
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
    
    user_id = ''
    if 'user_id' in session:
        user_id = session['user_id'] 
        status_response = api_request('GET', f'users/status?user_id={user_id}')
        status = status_response.get('status', 0) if status_response and status_response.get('success') else 0
        
        is_fav = is_favourite(anime_id)
        user_anime_history = get_user_anime_history(anime_id)
    else:
        status = 0
        is_fav = False
        user_anime_history = None
        
    logging.debug(anime_data)
        
    return render_template("anime.html", anime=anime_data, user_status=status, is_favourite=is_fav, user_id=user_id, user_anime_history=user_anime_history)


@app.route('/anime/<anime_id>/edit')
@login_required
def anime_edit_page(anime_id):
    if get_user_status(session['user_id']) <= 1:
        return render_template("login.html")
    
    if not anime_id:
        return
    
    if anime_id == 'new':
        if get_user_status(session['user_id']) <= 2:
            return render_template("login.html")
        default_path = "/images/anime/default.jpg"
        anime_data = {'id': anime_id, 'title': 'Заголовок', 'description': 'Описание', 'episodes': 1, 'videos': [], 'image_url': default_path}
        return render_template("anime_edit.html", anime=anime_data, user_status=get_user_status(session['user_id']))
    
    response = api_request('GET', f'anime/details?anime_id={anime_id}')
    if response and response.get('success'):
        anime_data = response.get('anime', '')
    else: 
        return
    if anime_data == '':
        return
    
    anime_data = json.loads(anime_data)
    anime_data['id'] = anime_id
        
    image_path = f'/images/anime/{anime_data["id"]}.jpg'
    default_path = "/images/anime/default.jpg"

    anime_data['image_url'] = image_path if check_file(image_path) else default_path    
        
    return render_template("anime_edit.html", anime=anime_data, user_status=get_user_status(session['user_id']))


@app.route('/update_anime', methods=['POST'])
def update_anime():
    if 'user_id' not in session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 403
    
    if get_user_status(session['user_id']) <= 1:
        return jsonify({'success': False, 'error': 'Insufficient privileges'}), 403
    
    data = request.get_json()
    anime_id = data.get('id', '')
    if anime_id == 'new':
        if get_user_status(session['user_id']) <= 2:
            return jsonify({'success': False, 'error': 'Insufficient privileges'}), 403
        add_response = api_request('POST', 'anime/add', data)
        if add_response and add_response.get('success'):
            image_url = data.get('image_url', '')
            filename = os.path.basename(image_url)
            src_path = os.path.join(app.static_folder, 'images', 'anime', 'uploads', filename)
            logging.debug(src_path)
            logging.debug(os.path.exists(src_path))
            if os.path.exists(src_path) and allowed_file_image(src_path):
                new_filename = f"{add_response.get('db_id','blank')}.jpg"
                dest_path = os.path.join(app.static_folder, 'images', 'anime', new_filename)
                
                shutil.move(src_path, dest_path)

            new_anime_counter.inc()

            return jsonify({"success": True, "id": add_response.get('db_id', 'search')})
        return jsonify({"success": False})
    else:
        update_response = api_request('POST', 'anime/update', data)
        if update_response and update_response.get('success'):
            image_url = data.get('image_url', '')
            filename = os.path.basename(image_url)
            src_path = os.path.join(app.static_folder, 'images', 'anime', 'uploads', filename)
            logging.debug(src_path)
            logging.debug(os.path.exists(src_path))
            if os.path.exists(src_path) and allowed_file_image(src_path):
                new_filename = f"{anime_id}.jpg"
                dest_path = os.path.join(app.static_folder, 'images', 'anime', new_filename)
                
                shutil.move(src_path, dest_path)
            return jsonify({"success": True})
    return jsonify({"success": False})


@app.route('/get_favourite', methods=['GET'])
def get_favourite():
    if 'user_id' not in session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 403
    user_id = session['user_id']
    fav_response = api_request('GET', f'users/favourites?user_id={user_id}')
    favorite_anime_ids = fav_response.get('anime_ids', '[]') if fav_response and fav_response.get('success') else '[]'
    
    data = json.loads(favorite_anime_ids)
    return data


def is_favourite(anime_id):
    user_id = session['user_id']
    fav_response = api_request('GET', f'users/favourites?user_id={user_id}')
    favorite_anime_ids = fav_response.get('anime_ids', '[]') if fav_response and fav_response.get('success') else '[]'
    
    data = json.loads(favorite_anime_ids)
    return anime_id in data


def get_user_anime_history(anime_id):
    user_id = session['user_id']
    history_response = api_request('GET', f'users/history?user_id={user_id}')
    anime_history = history_response.get('history', {}) if history_response and history_response.get('success') else None
    if anime_history is None:
        return None
    data = json.loads(anime_history)
    return data.get(anime_id, None)


@app.route('/add_favourite', methods=['POST'])
def add_favourite():
    if 'user_id' not in session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 403
    anime_id = request.json
    response = api_request('POST', 'users/favourites/add', {
        'user_id': session['user_id'],
        'anime_id': anime_id
    })
    if response and response.get('success'):
        return jsonify({"success": True})
    return jsonify({"success": False})


@app.route('/remove_favourite', methods=['POST'])
def remove_favourite():
    if 'user_id' not in session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 403
    anime_id = request.json
    response = api_request('POST', 'users/favourites/remove', {
        'user_id': session['user_id'],
        'anime_id': anime_id
    })
    if response and response.get('success'):
        return jsonify({"success": True})
    return jsonify({"success": False})


@app.route('/anime/search')
def anime():
    return render_template('search.html')


@app.route('/search_anime', methods=['GET'])
def search_anime():
    query = request.args.get('q', '').strip()

    if not query:
        return jsonify([])
    response = api_request('GET', f'anime/search?query={query}')
    
    if response and response.get('success'):
        return jsonify(response.get('anime_ids', [])) 

    return jsonify([]) 


@app.route('/get_all_anime', methods=['GET'])
def get_all_anime():
    response = api_request('GET', f'anime/search/all')
    
    if response and response.get('success'):
        return jsonify(response.get('anime_ids', [])) 

    return jsonify([]) 


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


@app.route('/delete_anime', methods=['POST'])
def delete_anime():
    if 'user_id' not in session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 403
    
    if get_user_status(session['user_id']) <= 2:
        return jsonify({'success': False, 'error': 'Insufficient privileges'}), 403
    
    data = request.get_json()
    
    update_response = api_request('POST', 'anime/delete', data)
    if update_response and update_response.get('success'):
        return jsonify({"success": True})
    return jsonify({"success": False})


@app.route('/login', methods=['GET', 'POST'])
def login():
    if 'user_id' in session:
        return redirect(url_for('index'))
    if request.method == 'POST':
        # API call to backend for authentication
        response = api_request('POST', 'auth/login', {
            'login': request.form['login'],
            'password_hash': hash_password(request.form['password'])
        })
        
        if response and response.get('success'):
            pwd = response.get('hashed_password', '')
            if bcrypt.checkpw(request.form['password'].encode('utf-8'), pwd.encode('utf-8')):
                session['user_id'] = response['user_id']
                return redirect(url_for('index'))
    return render_template('login.html')


@app.route('/register', methods=['GET', 'POST'])
def register():
    if 'user_id' in session:
        return redirect(url_for('index'))
    if request.method == 'POST':
        # API call to backend for user registration
        response = api_request('POST', 'auth/register', {
            'login': request.form['login'],
            'email': request.form['email'],
            'password_hash': hash_password(request.form['password'])
        })
        
        if response and response.get('success'):
            session['user_id'] = response['user_id']
            new_users_counter.inc()
            return redirect(url_for('index'))
    return render_template('register.html')


@app.route('/logout')
@login_required
def logout():
    session.pop('user_id', None)
    return redirect(url_for('login'))


@app.route('/chat')
@login_required
def chat():
    return render_template('chat.html', user_banned=is_banned(session['user_id']), user_status=get_user_status(session['user_id']))


def get_username(user_id):
    if 'user_id' not in session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 403
    redis_conn = app.config['SESSION_REDIS']
    username = redis_conn.get(f"user:{user_id}:name")
    if username:
        return username.decode('utf-8')

    response = api_request('GET', f'users/name?user_id={user_id}')
    if response and response.get('success'):
        user_name = response.get('name')

        redis_conn.set(f"user:{user_id}:name", user_name)
        return user_name

    return 'Anonim' 


@app.route('/ban_user', methods=['POST'])
def ban_user():
    if 'user_id' not in session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 403
    
    if get_user_status(session['user_id']) <= 2:
        return jsonify({'success': False, 'error': 'Insufficient privileges'}), 403
    
    target_user_id = request.json.get('user_id','')
    target_flag = request.json.get('flag', False)
    ban_response = api_request('POST', f'users/ban', data={
        'user_id': target_user_id,
        'flag': target_flag
    })
    if ban_response and ban_response.get('success'):
        banned_users_counter.inc()
        return jsonify({'success': True})
    return jsonify({'success': False, 'error': 'Failed to ban user'}), 500


@app.route('/delete_message', methods=['POST'])
def delete_message():
    if 'user_id' not in session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 403
    
    if get_user_status(session['user_id']) <= 2:
        return jsonify({'success': False, 'error': 'Insufficient privileges'}), 403
    
    message_id = request.json.get('message_id','')
    ban_response = api_request('POST', f'chat/messages/remove', data={
        'message_id': message_id
    })
    if ban_response and ban_response.get('success'):
        deleted_messages_counter.inc()
        return jsonify({'success': True})
    return jsonify({'success': False, 'error': 'Failed to delete message'}), 500


def is_banned(user_id):
    if 'user_id' not in session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 403
    user_id = session['user_id']
    banned_response = api_request('GET', f'users/banned?user_id={user_id}')
    if banned_response and banned_response.get('success'):
        result = banned_response.get('banned', False)
        return result
    else:
        return False
    
    
def get_user_status(user_id):
    status_response = api_request('GET', f'users/status?user_id={user_id}')
    status = status_response.get('status', 0) if status_response and status_response.get('success') else 0
    return status


@app.route('/get_messages')
def get_messages():
    if 'user_id' not in session:
        return jsonify([]) 
    
    last_timestamp = request.args.get('after', 0) 
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
    if is_banned(session['user_id']):
        return jsonify({'success': False, 'error': 'User banned from chat'}), 403

    message_text = request.json.get('message')
    if not message_text:
        return jsonify({'success': False, 'error': 'Empty message'}), 400

    response = api_request('POST', 'chat/send', data={
        'user_id': session['user_id'],
        'message': message_text,
        'timestamp_ms': int(time.time() * 1000)  # Add milliseconds timestamp
    })

    if response and response.get('success'):
        messages_counter.inc()
        return jsonify({'success': True})
    return jsonify({'success': False, 'error': 'Failed to send message'}), 500