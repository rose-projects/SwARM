#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import subprocess
import re

with open("BOM-split.csv", encoding="iso8859-15") as f:
    print("Not on Farnell:")
    part_dict = {}
    for line in f.readlines():
        values = line.split("\t")
        if len(values) < 7 :
            print(line)
            continue
        value = values[6]
        if "Fa : " not in value:
            print(line.strip())
            continue
        code = value.replace("Fa : ", "").strip()
        re.sub('\s+','', code)
        part_dict[code] = values[5]

for key in part_dict.keys():
    a = subprocess.Popen(['curl', "http://fr.farnell.com/webapp/wcs/stores/servlet/AjaxSearchLookAhead?storeId=10160&catalogId=15001&langId=-2&searchTerm=%s&selectedCategoryId=" % key],stdout=subprocess.PIPE)
    b = a.communicate()[0]
    if not b'searchResultProductList' in b:
        print("Error with: %s" % key)
        continue
    print("")
    print("%08s : %s"%(key, part_dict[key]))
    print((b.split(b'searchResultProductList">')[2].split(b"</a>")[0]).decode("utf-8"))
    print("http://fr.farnell.com/webapp/wcs/stores/servlet/Search?exaMfpn=true&mfpn=%s"%key)
