import pandas as pd
import numpy as np

df = pd.DataFrame(pd.date_range(start='2022-04-1',
                  end='2022-07-28', freq='10Min'), columns=['timestamp'])

df['timestamp'] = df['timestamp'].apply(
    lambda x: x.strftime("%Y-%m-%dT%H:%M:%SZ"))

df['temperature'] = np.random.randint(20, 24, size=(len(df)))
df['humidity'] = np.random.randint(42, 46, size=(len(df)))
df['prediction'] = 0

# for i in range(1, 2):
#     df.at[13000 + i, 'temperature'] = 40
#     df.at[13000 + i, 'humidity'] = 55
#     df.at[13000 + i, 'prediction'] = 1

# for i in range(1, 5):
#     df.at[13050 + i, 'temperature'] = 45
#     df.at[13050 + i, 'humidity'] = 60
#     df.at[13050 + i, 'prediction'] = 3

# for i in range(1, 4):
#     df.at[13100 + i, 'temperature'] = 55
#     df.at[13100 + i, 'humidity'] = 65
#     df.at[13100 + i, 'prediction'] = 2

print(df.head(15))

df.to_csv(path_or_buf="./training/sensors.csv", index=False)
