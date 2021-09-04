import csv

lat_lng = []
csv_list = []
src_file = "sample.csv" # ログファイル
dst_file = "output.csv" # 出力ファイル

# 全部読み込む
with open(src_file) as f:
  reader = csv.reader(f)
  for row in reader:
    csv_list.append(row)
f.close()

# 緯度・軽度のとこだけ読み込む
lat_lng.append(["latitude", "longitude"])
for index in range(len(csv_list)):
  if(csv_list[index][1] == "GPS:Sats"): # 条件：適宜変える
    lat_lng.append(csv_list[index + 1][1:3]) # [1:3]: lat, lng

# 出力
f = open(dst_file, "w")
writer = csv.writer(f, lineterminator='\n') # 改行
writer.writerows(lat_lng) # 改行ごとに行を生成
f.close()