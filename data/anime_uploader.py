import os
from dotenv import load_dotenv
import requests
import json
import csv

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

def download_image(url, filename):
    """Download image from URL and save to filename"""
    try:
        response = requests.get(url, stream=True)
        if response.status_code == 200:
            with open("/home/anime-watcher/frontend/static/images/anime/" + filename, 'wb') as f:
                for chunk in response.iter_content(1024):
                    f.write(chunk)
            return True
        else:
            print(f"Failed to download image from {url} - Status code: {response.status_code}")
            return False
    except Exception as e:
        print(f"Error downloading image: {e}")
        return False

# Read from CSV instead of JSON
with open('anime_data.csv', 'r', encoding='utf-8') as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        print(f"Sending {row['title']}")
        
        # Prepare the payload
        payload = {
            'title': row['title'],
            'description': row['description'],
            'episodes': int(row['episodes']),  # Convert to int
            'videos': []
        }
        
        # Make API request
        response = api_request('POST', 'anime/add', data=payload)
        
        if response and response.get('success'):
            db_id = response.get('db_id')
            print(f"{row['title']} uploaded with ID {db_id}")
            
            # Download and save the image
            if db_id and row['image_url']:
                image_filename = f"{db_id}.jpg"
                if download_image(row['image_url'], image_filename):
                    print(f"Image saved as {image_filename}")
                else:
                    print(f"Failed to download image for {row['title']}")
        else:
            print(f"Failed to upload {row['title']}")