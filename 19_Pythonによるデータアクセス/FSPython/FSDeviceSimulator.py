#concurrent.futures　による複数プロセスで、複数デバイスの動作を実行
#注意：Ctrl-Cでの停止が動かないので、コマンドプロンプトごと強制終了しかない

# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.
import concurrent.futures as confu

import os
import random
import time
import math
import re
from azure.iot.device import IoTHubDeviceClient, Message

# The device connection authenticates your device to your IoT hub. The connection string for 
# a device should never be stored in code. For the sake of simplicity we're using an environment 
# variable here. If you created the environment variable with the IDE running, stop and restart 
# the IDE to pick up the environment variable.
#
# You can use the Azure CLI to find the connection string:
# az iot hub device-identity show-connection-string --hub-name {YourIoTHubName} --device-id MyNodeDevice --output table

CONNECTION_STRING = [
    "デバイス接続文字列１",
    "デバイス接続文字列２",
    "デバイス接続文字列３", #以下必要に応じて追加可能
]

# Define the JSON message to send to IoT Hub.
TEMPERATURE = 20.0
HUMIDITY = 60
#MSG_TXT = '{{"temperature": {temperature},"humidity": {humidity}}}'
MSG_TXT = '{{"params":{{"sensor":"temp","espvalue": {temperature}}},"Dev":"{deviceName}","Id":{seqNo}}}'


def run_telemetry_sample(connectionString):
    # This sample will send temperature telemetry every second
    print("IoT Hub device sending periodic messages")
    seqNo = 0

    deviceName = re.findall('DeviceId=(.[^;]*);',connectionString)[0]
    client = IoTHubDeviceClient.create_from_connection_string(connectionString)
    client.connect()

    a1 = random.uniform(5,15)               #振幅 ℃
    T1 = random.uniform(15*60, 60*60)       #周期　sec
    p1 = T1 * random.random()               #位相　sec

    a2 = random.uniform(0,5)                #振幅
    T2 = random.uniform(1*60, 5*60)         #周期
    p2 = T2 * random.random()               #位相

    time.sleep(random.uniform(0,60))   #開始時間をランダムにばらつかせる
    print("start " + deviceName)
    
    t0= time.time()

    while True:
        # Build the message with simulated telemetry values.
        t = time.time() - t0
        temperature = 15 + a1* math.sin( (t+p1)*2*math.pi/T1) + a2* math.sin( (t+p2)*2*math.pi/T2)
        #humidity = HUMIDITY + (random.random() * 20)
#        msg_txt_formatted = MSG_TXT.format(temperature=temperature, humidity=humidity)
        msg_txt_formatted = MSG_TXT.format(temperature=temperature,deviceName=deviceName,seqNo=seqNo)
        message = Message(msg_txt_formatted)

        # Add a custom application property to the message.
        # An IoT hub can filter on these properties without access to the message body.
        #if temperature > 30:
        #    message.custom_properties["temperatureAlert"] = "true"
        #else:
        #    message.custom_properties["temperatureAlert"] = "false"

        # Send the message.
        print("Sending message: {}".format(message))
        client.send_message(message)
        print("Message successfully sent")
        time.sleep(10)
        seqNo +=1 



def main():
    print("IoT Hub Quickstart #1 - Simulated device")
    print("Press Ctrl-C to exit")

    # Instantiate the client. Use the same instance of the client for the duration of
    # your application
    ####client = IoTHubDeviceClient.create_from_connection_string(CONNECTION_STRING)
    # Run Sample
    try:
#        with confu.ThreadPoolExecutor(max_workers=os.cpu_count()) as executor:
        with confu.ThreadPoolExecutor() as executor:
            futures = [executor.submit(run_telemetry_sample, cs) for cs in CONNECTION_STRING]
            for future in confu.as_completed(futures):
                print(future.result())
    except KeyboardInterrupt:
        print("IoTHubClient sample stopped by user")
    finally:
        # Upon application exit, shut down the client
        print("Shutting down IoTHubClient")

if __name__ == '__main__':
    main()