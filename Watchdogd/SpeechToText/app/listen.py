import requests
import sounddevice
import subprocess
import os
import json
import queue
import numpy as np
import paho.mqtt.client as mqtt

from vosk import Model, KaldiRecognizer

VOSK_MODEL_PATH = "vosk-model"
SAMPLE_RATE = 16000
TRIGGER_WORDS = ["athéna", "hermès"]  # à adapter

# File tampon audio
audio_q = queue.Queue()

def publish_mqtt(chaine):
    print("[LOG] Envoi MQTT...")
    client = mqtt.Client()
    client.connect( os.getenv("ABLS_MQTT_BROKER", "localhost"), int(os.getenv("ABLS_MQTT_PORT", "1883")), 60)
    client.publish( os.getenv("ABLS_MQTT_TOPIC", "defaultopic"), chaine)
    client.disconnect()
    print("[LOG] Message MQTT envoyé.")

def Send_to_mistral ( texte ):
    print("[LOG] Preparing Mistral Request", texte )
    body = { "model": "mistral-small-latest",
             "messages": [ { "role": "user", "content": texte } ],
           }
    headers = { 'Authorization': f'Bearer '+ os.getenv("ABLS_CLEF_MISTRAL", "default_key"), 'Content-Type': 'application/json' }
    # Effectuer la requête POST
    response = requests.post('https://api.mistral.ai/v1/chat/completions', json=body, headers=headers)
    # Vérifier la réponse
    if response.status_code == 200:
        result = response.json()
        print(result)
    else:
        print("Erreur:", response.status_code, response.text)
    return response

def main():
    print("[LOG] Chargement du modèle Vosk...")
    model_vosk = Model(VOSK_MODEL_PATH)
    rec = KaldiRecognizer(model_vosk, SAMPLE_RATE ) #'["Alexa", "Amboise", "Lili"]')

    # Lancer parec en subprocess
    print("[LOG] Démarrage de l'écoute via parec...")
    parec_cmd = [
        "parec",
        "--format=s16le",
        "--rate=16000",
        "--channels=1",
        "--process-time-msec=20",
        "--device=" + os.getenv("PULSE_SOURCE", "default")
    ]
    print("[LOG] Commande parec_cmd :", parec_cmd)
    process = subprocess.Popen(parec_cmd, stdout=subprocess.PIPE)
    while True:
        data = process.stdout.read(1024)
        if not data:
            break

        if rec.AcceptWaveform(data):
            result_json = rec.Result()
            result = json.loads(result_json)
            print("[LOG] Résultat JSON Vosk complet :")
            print(json.dumps(result, indent=2))
            text = result.get("text", "").lower().strip()
            if not text:
                continue

            print(f"[LOG] Vosk a entendu : '{text}'")
    
            if any(word in text for word in TRIGGER_WORDS):
                print("[LOG] Mot-clé détecté, enregistrement appel MistralAI...")
                response = Send_to_mistral ( "Transforme le texte '"+ text +"' en 'intention' en te basant sur les champs 'examples' fournis dans la base de correlation publiée sur le site https://static.abls-habitat.fr/intents.json , et ne répond qu'avec le champ 'intention' associé. Si tu ne trouves pas de correspondance, réponds seulement 'inconnu'." );
                if response.status_code == 200:
                    result = response.json()
                    publish_mqtt(json.dumps(result))
                else:
                    print("Erreur:", response.status_code, response.text)

if __name__ == "__main__":
    main()

