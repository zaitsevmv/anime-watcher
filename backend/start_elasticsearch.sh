docker run -d \
  --name elasticsearch \
  -p 9200:9200 \
  -p 9300:9300 \
  -v ~/es-data:/data/elastic_db \
  docker.elastic.co/elasticsearch/elasticsearch:8.12.0