import re
import sys
import json

file_path_from_search = sys.argv[1]
file_path_from_set = sys.argv[2]

hash_str = ""
with open(file_path_from_search) as file:
    lines = file.read()
    hash_str = re.search("job\d*", lines)[0]

with open(file_path_from_set, "r") as file:
    data = json.load(file)

with open(file_path_from_set, "w") as file:
    data['hash_folder'] = hash_str
    json.dump(data, file, indent=4)