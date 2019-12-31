#!/bin/bash

# Domophone IP
IP=$1
# API Key setzen
# APIKEY="B387-A832-73E1-14A0";
APIKEY=$2
# ./open.sh 127.0.0.2 B387-A832-73E1-14A0

###################################################################################################################
# ab hier nichts ändern
# die URL
URL="https://$IP/webapi.cgi"
# mein Test
#URL="http://$IP:8000/t8161mt8"

# so lassen
ClientID="439A557E991F490D";
# so lassen
ClientName="WebClientSample";
# so lassen
HEADER="Content-Type: application/x-www-form-urlencoded"
PARAMS_OPEN_GATEWAY="apikey=$APIKEY&clientId=$ClientID&clientName=$ClientName&clientNameSize=15&action=1&param1=4"
PARAMS_OPEN_GATE="apikey=$APIKEY&clientId=$ClientID&clientName=$ClientName&clientNameSize=15&action=1&param1=5"


echo "Now I send OPEN_GATEWAY"
curl -v -k -H "$HEADER" -H "User-Agent:" -d "$PARAMS_OPEN_GATEWAY" "$URL"
echo "Now I send OPEN_GATE"
curl -v -k -H "$HEADER" -H "User-Agent:" -d "$PARAMS_OPEN_GATE" "$URL"
####################################################################################################################
