import transaction
from ZODB import DB
from ZODB.FileStorage import FileStorage
from ZODB.blob import Blob
from persistent import Persistent
from persistent.mapping import PersistentMapping


class Video(Persistent):
    def __init__(self, blob):
        self.blob = blob
        

class VideoDatabase:
    def __init__(self, db_path='data.fs'):
        self.storage = FileStorage(db_path)
        self.db = DB(self.storage)
        self.connection = self.db.open()
        self.root = self.connection.root
        
        if not hasattr(self.root, 'videos'):
            self.root.videos = PersistentMapping()
            transaction.commit()
        
    def add_video(self, file_path, video_hash):
        blob = Blob()
        with blob.open(file_path) as f:
            f.write(file_path.read())

        self.root.videos[video_hash] = Video(blob)
        transaction.commit()
    
    def delete_video(self, video_hash):
        if video_hash in self.root.videos:
            del self.root.videos[video_hash]
            transaction.commit()
    
    def get_video(self, video_hash):
        return self.root.videos.get(video_hash, None)
    
    def close(self):
        self.connection.close()
        self.db.close()