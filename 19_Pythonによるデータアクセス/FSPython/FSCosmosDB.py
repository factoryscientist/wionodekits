# CosmosDBの読み込み
from azure.cosmos import exceptions, CosmosClient, PartitionKey

# CosmosDB　の　URIとプライマリキー
endpoint = "cosmosDBのURI"
key = 'CosmosDBのプライマリキー'


client = CosmosClient(endpoint, key)
database_name = 'fsdatabase'    #DB名を設定する
database = client.create_database_if_not_exists(id=database_name)
container_name = 'fscontainer'  #コンテナ名を設定する
container = database.create_container_if_not_exists(
     id=container_name,
     partition_key=PartitionKey(path="/device"),
     offer_throughput=400
)

# クエリの例
# query = "SELECT * FROM c WHERE c.time > '2022-03-31' order by c._ts DESC offset 0 limit 1"
query = "SELECT * FROM c order by c._ts DESC  offset 0 limit 1000"

items = list(container.query_items( query=query, enable_cross_partition_query=True ))
request_charge = container.client_connection.last_response_headers['x-ms-request-charge']
print('Query returned {0} items. Operation consumed {1} request units'.format(len(items), request_charge))
items[0]


# 複数系列　まとめてグラフ化
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from datetime import datetime as dt

df=pd.DataFrame(items)

values={}
times={}
devices=np.sort(df.device.unique())
for i in devices:
    values[i]=df[df.device==i].value
    times[i]=[dt.strptime(j,'%Y-%m-%dT%H:%M:%S.%f0z') for j in df[df.device==i].time ]
    plt.plot(times[i],values[i],label=i)

plt.legend()
plt.gcf().autofmt_xdate()
plt.show()

# CSV出力
df.to_csv(path_or_buf='cosmos.csv',encoding='shift_jis')