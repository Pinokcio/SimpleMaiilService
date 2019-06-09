#-*- coding: utf-8 -*-
import json
import sys
import io
import os

# export PYTHONIOENCODING=utf-8

jsonfile = "list_mail-" + sys.argv[1] +".json"

with open(jsonfile, 'r', -1, 'utf-8') as file:
    jsondata = json.load(file)

with open(jsonfile, 'w') as file:
    json.dump(jsondata, file, ensure_ascii=False, indent = 4)
