docker run -d \
  --name elasticsearch \
  -p 9200:9200 \
  -p 9300:9300 \
  -v ~/es-data:/data/elastic_db \
  -e "discovery.type=single-node" \
  -e "ELASTIC_USERNAME=elastic" \
  -e "ELASTIC_PASSWORD=secret" \
  docker.elastic.co/elasticsearch/elasticsearch:8.12.0