FROM python:3.11-slim

# Dépendances système pour audio + Whisper
RUN apt-get update && apt-get install -y \
    build-essential \
    ffmpeg mpg123 libsndfile1\
    libportaudio2 \
    pulseaudio-utils \
    && apt-get clean

# Copie du code
WORKDIR /app
COPY app/ /app/

# Installation des dépendances Python
RUN pip install --no-cache-dir --upgrade pip && \
    pip install --no-cache-dir -r requirements.txt

# Exposition du port MQTT (optionnel, pour debug)
#EXPOSE 1883

CMD ["python", "listen.py"]
