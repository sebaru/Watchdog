#/bin/sh
echo Shelly $1: Setting MQTT server to $2
curl -X POST -d '{"id":1, "method":"Mqtt.SetConfig", "params":{"config":{"enable":true, "server":"$2:1883"}}}' http://$1/rpc
echo
echo Shelly $1: Rebooting
curl -X POST -d '{"id":1, "method":"Shelly.Reboot"}' http://$1/rpc
echo
sleep 5
echo Shelly $1: Getting Config
curl -X POST -d '{"id":1, "method":"Mqtt.GetConfig"}' http://$1/rpc
echo
