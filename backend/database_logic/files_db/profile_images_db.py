import transaction
from ZODB import DB
from ZODB.FileStorage import FileStorage
from ZODB.blob import Blob
from persistent import Persistent
from persistent.mapping import PersistentMapping


class ProfileImages(Persistent):
    def __init__(self, blob):
        self.blob = blob
        

class ProfileImagesDatabase:
    def __init__(self, db_path='data.fs'):
        self.storage = FileStorage(db_path)
        self.db = DB(self.storage)
        self.connection = self.db.open()
        self.root = self.connection.root
        
        if not hasattr(self.root, 'profile_images'):
            self.root.profile_images = PersistentMapping()
            transaction.commit()
        
    def add_profile_image(self, file_path, user_id):
        blob = Blob()
        with blob.open(file_path) as f:
            f.write(file_path.read())

        self.root.profile_images[user_id] = ProfileImages(blob)
        transaction.commit()
    
    def delete_profile_image(self, user_id):
        if user_id in self.root.profile_images:
            del self.root.profile_images[user_id]
            transaction.commit()
    
    def get_profile_image(self, user_id):
        return self.root.profile_images.get(user_id, None)
    
    def close(self):
        self.connection.close()
        self.db.close()