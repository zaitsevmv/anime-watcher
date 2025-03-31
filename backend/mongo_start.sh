docker run -d --name mongodb -p 27017:27017 -v ~/mongo-data:/data/db -e MONGO_INITDB_ROOT_USERNAME=admin -e MONGO_INITDB_ROOT_PASSWORD=secret mongo
