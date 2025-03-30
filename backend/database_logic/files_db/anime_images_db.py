import transaction
from ZODB import DB
from ZODB.FileStorage import FileStorage
from ZODB.blob import Blob
from persistent import Persistent
from persistent.mapping import PersistentMapping


class AnimeImage(Persistent):
    def __init__(self, blob):
        self.blob = blob
        

class AnimeImageDatabase:
    def __init__(self, db_path='data.fs'):
        self.storage = FileStorage(db_path)
        self.db = DB(self.storage)
        self.connection = self.db.open()
        self.root = self.connection.root
        
        if not hasattr(self.root, 'anime_images'):
            self.root.anime_images = PersistentMapping()
            transaction.commit()
        
    def add_anime_image(self, file_path, anime_hash):
        blob = Blob()
        with blob.open(file_path) as f:
            f.write(file_path.read())

        self.root.anime_images[anime_hash] = AnimeImage(blob)
        transaction.commit()
    
    def delete_anime_image(self, anime_hash):
        if anime_hash in self.root.anime_images:
            del self.root.anime_images[anime_hash]
            transaction.commit()
    
    def get_anime_image(self, anime_hash):
        return self.root.anime_images.get(anime_hash, None)
    
    def close(self):
        self.connection.close()
        self.db.close()