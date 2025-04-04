import os
from dotenv import load_dotenv
import requests
import json

load_dotenv()
API_BASE_URL = os.getenv('API_BASE_URL')

def api_request(method, endpoint, data=None, headers=None):
    """Helper function for making API calls"""
    url = f"{API_BASE_URL}/{endpoint}"
    try:
        response = requests.request(method, url, json=data, headers=headers)
        return response.json()
    except Exception as e:
        print(f"API Error: {e}")
        return None
    

with open('anime.json', 'r') as file:
    data = json.load(file)

for a in data:
    print(f"Sending {a['title']}")
    payload = {
        'title': a['title'],
        'description': a['description'],
        'videos': [0]*a['episodes']
    }
    response = api_request('POST', 'anime/add', data={
        'title': a['title'],
        'description': a['description'],
        'videos': [0]*a['episodes']
    })
    if response and response.get('success'):
        print(f"{a['title']} uploaded")
    break