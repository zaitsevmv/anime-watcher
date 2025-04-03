from flask import Flask, render_template, request, jsonify, session, redirect, url_for
from flask_socketio import SocketIO, send
import requests  # For making API calls
from dotenv import load_dotenv
import os
import time
import logging

load_dotenv()  # Load environment variables from .env file

app = Flask(__name__)
app.config['SECRET_KEY'] = os.getenv('FLASK_SECRET_KEY')
API_BASE_URL = os.getenv('API_BASE_URL')
socketio = SocketIO(app)

logging.basicConfig(filename="debug.log", level=logging.DEBUG, format="%(asctime)s - %(levelname)s - %(message)s")


def api_request(method, endpoint, data=None, headers=None):
    """Helper function for making API calls"""
    url = f"{API_BASE_URL}/{endpoint}"
    try:
        response = requests.request(method, url, json=data, headers=headers)
        return response.json()
    except Exception as e:
        print(f"API Error: {e}")
        return None

@app.route('/')
def index():
    """Fetch user's favorite anime IDs and pass them to the template."""
    if 'user_id' not in session:
        return redirect(url_for('login'))

    user_id = session['user_id']
    fav_response = api_request('GET', f'users/favorites/{user_id}')
    # Make sure we have a list of anime IDs; otherwise, an empty list
    favorite_anime_ids = fav_response.get('anime_ids', []) if fav_response and fav_response.get('success') else []

    return render_template('index.html', anime_ids=favorite_anime_ids)

@app.route('/anime')
def anime():
    """Render the anime search page."""
    return render_template('anime.html')

@app.route('/search_anime')
def search_anime():
    """API route to search anime based on user input (live search)."""
    query = request.args.get('q', '').strip()

    if not query:
        return jsonify([])  # Return empty if no search term

    # Call backend search API
    response = api_request('GET', f'anime/search?query={query}')
    
    if response and response.get('success'):
        return jsonify(response.get('anime_ids', []))  # Return list of found anime

    return jsonify([])  # Return empty on error

# Authentication routes
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

@app.route('/get_messages')
def get_messages():
    """Fetch messages after a given timestamp from the backend."""
    last_timestamp = request.args.get('after', 0)  # Default to 0 if no timestamp is provided
    response = api_request('GET', f'chat/messages?after={last_timestamp}')
    
    logging.debug(f"API response: {response}")
    
    def get_username(user_id):
        """Fetch user_name from user_id using an API call."""
        response = api_request('GET', f'users?name={user_id}')
        if response and response.get('success'):
            return response.get('user_name', user_id)
        return user_id

    if response and response.get('success'):
        messages_list = response.get('messages', [])
        for msg in messages_list:
            msg['user_name'] = get_username(msg['user_id'])
    return jsonify([])  # Return empty list if there’s an issue

@app.route('/send_message', methods=['POST'])
def send_message():
    """Send a new message to the backend."""
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