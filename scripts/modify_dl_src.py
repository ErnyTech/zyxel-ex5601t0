#!/usr/bin/python3

import csv
import sys
import re
import os

def process_one_makefile_hash(data):

    update = 0
    with open(data['path'], "r") as f:
        contents = f.readlines()

    for index in range(len(contents)):
#        print("index = ",index,"content = ",contents[index])
        if re.findall(r"^PKG_HASH", contents[index]) :
            print("Convert: ",data['path'],"  PKG_HASH")
            update=1  
            contents[index] = contents[index].replace("PKG_HASH","#PKG_HASH")
            break


    if update == 0:
        print("No PKG_HASH")
    if update == 1:
        with open(data['path'], "w") as f:
            contents = "".join(contents)
            f.write(contents)

def process_one_makefile(data):

    update = 0
    skip = 0;
    with open(data['path'], "r") as f:
        contents = f.readlines()


    for index in range(len(contents)):
#        print("index = ",index,"content = ",contents[index])
        if re.findall(r"@ZyXEL_SITE/", contents[index]) :
# We had done
            skip = 1
            break
        
        if re.findall(r"^PKG_SOURCE_PROTO", contents[index]) :
            update=1
            contents[index] = contents[index].replace("PKG_SOURCE_PROTO","#PKG_SOURCE_PROTO")


    if skip == 0 :
        for index in range(len(contents)):
    #        print("index = ",index,"content = ",contents[index])

            if re.findall(r"^PKG_SOURCE_URL", contents[index]) :
                update=1
                contents[index] = contents[index].replace("PKG_SOURCE_URL","#PKG_SOURCE_URL")
                contents.insert(index, data['newurl']+'\n')
                print("Convert: ",data['path'],"  ", data['newurl'])
                break
#special case for feeds/packages/lang/python/pypi.mk
            if re.findall(r"PKG_SOURCE_URL?", contents[index]) :
                update=1
                contents[index] = contents[index].replace("PKG_SOURCE_URL","#PKG_SOURCE_URL")
                contents.insert(index, data['newurl']+'\n')
                print("Convert: ",data['path'],"  ", data['newurl'])
                break


    if update == 0 and skip == 0:
        print("No PKG_SOURCE_URL")
    if update == 1:
        with open(data['path'], "w") as f:
            contents = "".join(contents)
            f.write(contents)


def process_makefile(db):
    for row in db:
        process_one_makefile(row)


def restore_back(row):
    cmd="git checkout -- "+ row['path']
    os.system(cmd)

def main(orig_args):
    with open("opensource_url.csv", newline='') as csvfile:
        db = csv.DictReader(csvfile)

        for row in db:
            print(row['path'], row['type'], row['newurl'])
            if row['type'] == 'url' :
                process_one_makefile(row)
            if row['type'] == 'hash' :
                process_one_makefile_hash(row)
#            restore_back(row)

if __name__ == "__main__":
#    print (type(sys.argv[1:]))
#    find_project_root_path()
    main(sys.argv)