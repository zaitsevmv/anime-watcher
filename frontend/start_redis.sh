docker run -d \
    --name redis \
    --network=anime-net \
    -p 192.168.0.2:6379:6379 redis
