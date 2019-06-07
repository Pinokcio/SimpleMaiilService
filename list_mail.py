#-*- coding: utf-8 -*-
import gmail
import sys
import io
import base64
import sys
import json
import urllib
import os
from collections import OrderedDict

reload(sys)
sys.setdefaultencoding('utf-8')

def list_mail(id) :
    jsonfile = "list_mail-" + id +".json"
    all_box_mail = g.inbox().mail()

    if os.path.isfile("./" + jsonfile) :
        with open(jsonfile, 'r+w') as file:
            jsondata = file.readlines()
            start_cnt = jsondata[len(jsondata)-2]
            rm_length = len(start_cnt)

            start_cnt = start_cnt.replace("\"Count\":", "")
            start_cnt, last_update = start_cnt.split(",")
            start_cnt = int(start_cnt)

            last_update = last_update.replace("\"update\":", "")

            last_time = jsondata[len(jsondata)-3]
            last_time = last_time.split("\"day\": ")
            last_time = last_time[1]
            last_time = last_time.split(",")
            last_time = last_time[0]
            last_time = int(last_time)

            file.seek(-rm_length-2, os.SEEK_CUR)

            cnt = 0

            for newmail in all_box_mail:
                newmail.fetch()
                time = str(unicode(newmail.sent_at))
                time = time.replace("-", "")
                time = time.replace(":", "")
                time = time.replace(" ", "")
                time = int(time)
                if time > last_time:
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
                    tmp["day"] = time
                    tmp["is_trash"] = 0
                    if str(unicode(newmail.flags)) == "['\\\\Seen']":
                        tmp["is_read"] = 1
                    else:
                        tmp["is_read"] = 0
                    tmp_cnt = start_cnt + cnt
                    file.write('"' + str(tmp_cnt) + '"' + ":")
                    json.dump(tmp, file, ensure_ascii=False)
                    file.write(",\n")

            if cnt != 0:
                cnt += start_cnt
                # file.seek(-1, os.SEEK_CUR)
                file.write('"Count"' + ":" + str(cnt) + "," + '"update"' + ":" + str(last_time) + "\n}")
        file.close()
        return

    else :
        with open(jsonfile, 'w') as file:
            file.write('{\n')
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
                file.write('"' + str(cnt) + '"' + ":")
                json.dump(tmp, file, ensure_ascii=False)
                file.write(',\n')
            file.write('"Count"' + ":" + str(cnt) + "," + '"update"' + ":" + "0\n}")
    file.close()
    return

# try:
g = gmail.login(sys.argv[1], sys.argv[2])
g.logged_in
# except  gmail.AuthenticationError:  # could not log in

list_mail(sys.argv[1])
g.logout
