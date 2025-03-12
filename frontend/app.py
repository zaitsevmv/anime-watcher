from flask import Flask, render_template, request, redirect, url_for, session, flash
from flask_socketio import SocketIO, send
import requests  # For making API calls
from dotenv import load_dotenv
import os

load_dotenv()  # Load environment variables from .env file

app = Flask(__name__)
app.config['SECRET_KEY'] = os.getenv('FLASK_SECRET_KEY')
API_BASE_URL = os.getenv('API_BASE_URL')
socketio = SocketIO(app)


def api_request(method, endpoint, data=None, headers=None):
    """Helper function for making API calls"""
    url = f"{API_BASE_URL}/{endpoint}"
    try:
        response = requests.request(method, url, json=data, headers=headers)
        return response.json()
    except Exception as e:
        print(f"API Error: {e}")
        return None
    
# Mock data for demonstration
anime_list = [
    {
        'id': 1,
        'title': 'Attack on Titan',
        'image': 'aot.jpg',
        'video': 'aot_ep1.mp4',
        'description': 'Humans fight giant humanoid creatures called Titans'
    },
    {
        'id': 2,
        'title': 'Demon Slayer',
        'image': 'demonslayer.jpg',
        'video': 'demonslayer_ep1.mp4',
        'description': 'A young boy becomes a demon slayer after his family is slaughtered'
    }
]

@app.route('/')
def index():
    # Display first anime for demonstration
    return render_template('index.html', anime=anime_list[0])

@app.route('/search')
def search():
    query = request.args.get('q', '')
    results = [anime for anime in anime_list if query.lower() in anime['title'].lower()]
    return render_template('search.html', results=results, query=query)

# Authentication routes
@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        # API call to backend
        response = api_request('POST', 'auth/login', {
            'username': request.form['username'],
            'password': request.form['password']
        })
        
        if response and response.get('success'):
            session['user_token'] = response['token']
            return redirect(url_for('index'))
        flash('Invalid credentials')
    return render_template('login.html')

@app.route('/register', methods=['GET', 'POST'])
def register():
    if request.method == 'POST':
        # API call to backend
        response = api_request('POST', 'auth/register', {
            'username': request.form['username'],
            'email': request.form['email'],
            'password': request.form['password']
        })
        
        if response and response.get('success'):
            flash('Registration successful! Please login.')
            return redirect(url_for('login'))
        flash('Registration failed')
    return render_template('register.html')

@app.route('/logout')
def logout():
    session.pop('user_token', None)
    return redirect(url_for('index'))

# Updated chat route with authentication
@app.route('/chat')
def chat():
    if 'user_token' not in session:
        return redirect(url_for('login'))
    return render_template('chat.html')

# Modified WebSocket handler with authentication
@socketio.on('message')
def handle_message(msg):
    if 'user_token' not in session:
        return
    # Store message via API
    api_request('POST', 'chat', {
        'message': msg,
        'user_token': session['user_token']
    }, headers={'Authorization': f"Bearer {session['user_token']}"})
    
    send({
        'message': msg,
        'user': session.get('username', 'Anonymous')
    }, broadcast=True)

# Add login_required decorator to protected routes as needed