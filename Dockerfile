FROM python:3.14-alpine

RUN apk add build-base

WORKDIR /app

COPY requirements.txt .

RUN pip install --no-cache-dir -r requirements.txt

COPY wsgi.py .

COPY frontend frontend

EXPOSE 8080

CMD ["python", "wsgi.py"]