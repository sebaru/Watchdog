#!/bin/sh
podman run --rm -it \
    --device /dev/snd \
    --group-add audio \
    --name abls-speech-to-mqtt \
    -v /etc/machine-id:/etc/machine-id:ro \
    -v ~/.config/pulse/cookie:/root/.config/pulse/cookie:ro \
    -v $XDG_RUNTIME_DIR/pulse:/run/user/1000/pulse \
    --env ABLS_MQTT_BROKER=satis \
    --env ABLS_MQTT_PORT=1883 \
    --env ABLS_MQTT_TOPIC=/speech/intent \
    --env PULSE_SERVER=unix:/run/user/1000/pulse/native \
    --env PULSE_SOURCE=alsa_input.usb-0b0e_Jabra_SPEAK_510_USB_745C4B657953021800-00.mono-fallback \
    --env ABLS_CLEF_MISTRAL=k4KJDQQ6EmdRR7p5WjqFzT9926npU4uN \
    abls-speech-to-mqtt:latest

#alsa_input.usb-046d_B525_HD_Webcam_5F15B500-00.mono-fallback

