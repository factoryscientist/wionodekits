# IoTHubユーザデータ処理
""" 
    ・この「モジュールにIoTHub がデータを受け取るたびに呼ばれる関数を記載する
    使用方法：
    ・以下のユーザ定義変数を　変更してから使用すること
        CONNECTION_STR
        ALEART_DEVICE_NAME
    ・必要に応じ　recieveUserData のデータ処理を変更すること
    ・デバイスからIoTHubへの送信メッセージフォーマットを変更した場合は、必要に応じparseUsrMsg2Dic の処理を変更すること
"""
import pandas as pd
import re                           
import ast
import datetime as dt
from FSC2DMessage import iothub_messaging_sample_run  #データレシーブ時に必要に応じアラートメッセージを　C2D送信する。

#　↓ CONNECTION_STR　に自分の IoTHub の「組込みのエンドポイント」の　「共有アクセスポリシー」を「service」を選択した時に表示される、
# 「イベントハブ互換エンドポイント」の文字列を入れてください
#　例：　CONNECTION_STR ="Endpoint=sb://iothub-ns-jinnouchii-5698076-7882ac2621.servicebus.windows.net/;SharedAccessKeyName=service;SharedAccessKey=8PXMpYiEF9cunSzhq/MshdQZGn0R1Tv+dTAze10JelE=;EntityPath=jinnouchiiothub"
 CONNECTION_STR ="自分の イベントハブ互換エンドポイント"


cols = [ "Dev","Id", "espvalue", "sensor", b'iothub-enqueuedtime' ]  #DataFrameの注目カラム名。
#　空のDataFrameでも　アクセスしたいカラム名を作っておかないとアクセス時にエラーを起こすため。
df = pd.DataFrame(columns=cols)     #　取得したデータ系列を必要行数保管しておくDataFrame

def recieveUserData(usrMsgStr,sysMsgDic):
    """ IoTHubに来たデータを受け取るユーザ定義処理
    Args:
        usrMsgStr (str): IoTHubに届いたメッセージのユーザ定義部分。　以下のような形式を想定
            {"params":{"sensor":"temp","espvalue": 19.28016494311027},"Dev":"jinnouchi07sim02","Id":4101}}

        sysMsgStr (str): IoTHubに届いたメッセージのシステム定義部分。以下のような形式を想定
            {b'iothub-connection-device-id': b'jinnouchi07sim02', b'iothub-connection-auth-method': b'{"scope":"device","type":"sas","issuer":"iothub","acceptingIpFilterRule":null}', b'iothub-connection-auth-generation-id': b'637854357009980941', b'iothub-enqueuedtime': 1650086500936, b'iothub-message-source': b'Telemetry', b'x-opt-sequence-number': 3533801, b'x-opt-offset': b'390857417808', b'x-opt-enqueued-time': 1650086500953}
    Raises:
        NA: 特に上げない

    Returns:
        NA: 何も返さない

    Note: 
        グローバル変数　の　df に　取得データを追加する。　不要になったデータ行は破棄する。
    """
    global df

    du = parseUsrMsg2Dic(usrMsgStr) # ユーザデータの解釈
    # print(du)

    #ds = parseSysMsg2Dic(sysMsgStr) # システムデータの解釈
    # print(ds)

    # dic = dict( **du, **ds )          # 2つの辞書を結合する
    dic = du | sysMsgDic

    #print(df[cols])
    # df から　が今受信した Dev の行を除く
    df=df[ df['Dev'] != du['Dev'] ]

    # df から、今受信した　dsiothub-enqueuedtime　より　1分超前のデータを削除する
    # df = df[df['iothub-enqueuedtime']>= ds['iothub-enqueuedtime'] - 60000]
    print("iothub-enqueuedtime ", sysMsgDic[b'iothub-enqueuedtime'])
    df = df[df[b'iothub-enqueuedtime']>= sysMsgDic[b'iothub-enqueuedtime'] - 60000]

    #　df　に　受信データを追加する
    df=df.append(dic, ignore_index=True)
    print(df[cols])
    #　dfの　espvalue　の最大値、最小値の差が　10度　を超えたらアラート c2d を発信
    dif = df['espvalue'].max() - df['espvalue'].min()
    if 10 < dif :
        iothub_messaging_sample_run()  # C2D発信
        print("!!! Alert sended !!!")
    
    print()
    return


def  parseUsrMsg2Dic(msgStr):
    """ IoTHubに来たユーザメッセージ文字列を 辞書型(dict)に変換する
    Args:
        msgstr (str): IoTHubに届いたメッセージ。以下のような形式を想定
        '{"params":{"sensor":"temp","espvalue": 21.680393137022627},"Dev":"jinnouchi07sim02","Id":2018}'
    Raises:
        NA: [特に上げない]
    Returns:
        dict : メッセージを変換した dict型 1レコード

    Note: 

    """
    msgStr = msgStr.strip()     #余計な前後の空白文字を除去

    #IoTHubメッセージのネストして不要な部分を削除し、DataFrameに変換しやすくする
    d = msgStr.replace('{"params":','').replace('},' , ',')
    dic = ast.literal_eval(d)

    return dic
    
def  parseSysMsg2Dic(msgStr):
    """ IoTHubに来たシステムメッセージ文字列を 辞書型(dict)に変換する
    Args:
        msgstr (str): IoTHubに届いたメッセージ。以下のような形式を想定
        '"params":{"sensor":"temp","espvalue": 19.28016494311027},"Dev":"jinnouchi07sim02","Id":4101'
    Raises:
        NA: [特に上げない]

    Returns:
        dict : メッセージを変換した dict型 1レコード

    Note: 

    """
    msgStr = msgStr.strip()     #余計な前後の空白文字を除去

    #IoTHubメッセージのネストして不要な部分を削除し、DataFrameに変換しやすくする
    d = re.sub("'{.*}'", "'DELETED'", msgStr) 
    d = d.replace("b'","'")
    dic = ast.literal_eval(d)
    
    return dic
