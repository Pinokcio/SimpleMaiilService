#-*- coding: utf-8 -*-
import gmail
import sys
import io
import base64
import json
import urllib
import os
from collections import OrderedDict
from subprocess import call

reload(sys)
sys.setdefaultencoding('utf-8')

def list_mail(id) :
    jsonfile = "list_mail-" + id +".json"
    all_box_mail = g.inbox().mail()
    re_flag = False

    if os.path.isfile("./" + jsonfile) :
        with open(jsonfile, 'r') as file:
            jsondata = json.load(file, object_pairs_hook=OrderedDict)
            start_cnt = jsondata['Count']
            last_time = jsondata[str(start_cnt)]['day']
            re_flag = True
            # "update" = str(last_time)

    with open(jsonfile, 'w+') as file:
        root_dict = OrderedDict()
        root_dict["Count"] = 0
        root_dict["update"] = 0
        cnt = 0
        for newmail in all_box_mail:
            newmail.fetch()
            cnt += 1
            tmp = OrderedDict()

            tmp["mail_id"] = newmail.message_id
            tmp["sub"] = newmail.subject
            full_name = newmail.fr

            if '<' in full_name:
                from_name, from_email = full_name.split('<')
                if from_name == "":
                    from_name = "Unknown"
            else :
                from_name = "Unknown"
                from_email = full_name

            from_name = from_name.replace("\"", "")
            from_name = from_name.rstrip()
            if "=?" in from_name:
                if "UTF" in from_name:
                    from_name = from_name[10:]
                    from_name = from_name.replace("?=", "")
                    from_name = from_name.replace("=","%")
                    from_name = urllib.unquote(from_name)
                elif "utf" in from_name:
                    from_name = from_name[10:]
                    from_name = from_name.replace("?=", "")
                    from_name = base64.decodestring(from_name)
            from_email = from_email.replace(">", "")
            tmp["from_name"] = from_name
            tmp['from_email'] = from_email
            # 시간 기호 삭제
            time = str(unicode(newmail.sent_at))
            time = time.replace("-", "")
            time = time.replace(":", "")
            time = time.replace(" ", "")
            tmp['day'] = int(time)

            tmp["is_trash"] = 0
            if str(unicode(newmail.flags)) == "['\\\\Seen']":
                tmp["is_read"] = 1
            else:
                tmp["is_read"] = 0
            # file.write('"' + str(cnt) + '"' + ":")
            if re_flag:
                if tmp['day'] > last_time:
                    jsondata[str(cnt)] = tmp
            else :
                root_dict[str(cnt)] = tmp
            # json.dump(tmp, file, ensure_ascii=False)
        if re_flag:
            jsondata["update"] = last_time
            jsondata["Count"] = cnt
            json.dump(jsondata, file, ensure_ascii=False)
        else:
            root_dict['Count'] = cnt
            json.dump(root_dict, file, ensure_ascii=False)
        # file.write("\n}")
        # file.write('"Count"' + ":" + str(cnt) + "," + '"update"' + ":" + "0\n}")
    file.close()
    return

# try:
g = gmail.login(sys.argv[1], sys.argv[2])
g.logged_in
# except  gmail.AuthenticationError:  # could not log in
list_mail(sys.argv[1])
g.logout
