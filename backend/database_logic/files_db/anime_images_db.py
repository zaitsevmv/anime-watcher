import transaction
from ZODB import DB
from ZEO import Client
from ZODB.FileStorage import FileStorage
from ZODB.blob import Blob
from persistent import Persistent
from persistent.mapping import PersistentMapping


class AnimeImage(Persistent):
    def __init__(self, blob):
        self.blob = blob
        

class AnimeImageDatabase:
    def __init__(self, zeo_address=('localhost', 8100)):
        self.storage = Client.ClientStorage(zeo_address)
        self.db = DB(self.storage)
        self.connection = self.db.open()
        self.root = self.connection.root
        
        if not hasattr(self.root, 'anime_images'):
            self.root.anime_images = PersistentMapping()
            transaction.commit()
        
    def add_anime_image(self, file_path, anime_id):
        blob = Blob()
        
        with open(file_path, 'rb') as source:
            with blob.open(file_path, 'w') as f:
                f.write(source.read())
        
        # Using anime_id (a string) directly as the key.
        self.root.anime_images[anime_id] = AnimeImage(blob)
        transaction.commit()
    
    def delete_anime_image(self, anime_id):
        if anime_id in self.root.anime_images:
            del self.root.anime_images[anime_id]
            transaction.commit()
    
    def get_anime_image(self, anime_id):
        return self.root.anime_images.get(anime_id, None)
    
    def close(self):
        self.connection.close()
        self.db.close()
