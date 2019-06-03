#-*- coding: utf-8 -*-
import gmail
import email.Header
import sys
import io
import base64
import sys
import json
import urllib
from collections import OrderedDict
mailData = []
reload(sys)
sys.setdefaultencoding('utf-8')

def list_mail(id) :
    all_box_mail = g.inbox().mail()
    list = []
    tmp = OrderedDict()
    jsonfile = "list_mail-" + id +".json"
    with open(jsonfile, 'w') as file:
        for newmail in all_box_mail:
            newmail.fetch()
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
            tmp["day"] = str(unicode(newmail.sent_at))
            tmp["is_trash"] = "false"
            if str(unicode(newmail.flags)) == "['\\\\Seen']":
                tmp["is_read"] = "true"
            else:
                tmp["is_read"] = "false"

            json.dump(tmp, file, ensure_ascii=False)
            file.write('\n')
            tmp = OrderedDict()


# def get_mail_body(msg_id) :
#     all_box_mail = g.inbox().mail()
#     while(1):
#         if msg_id == all_box_mail.message_id:
#             body = all_box_mail.body
#             print(body)
#             break;
#     with open('mail_body', 'w') as file:
#         json.dump(body, file, ensure_ascii=False)

try:
    g = gmail.login(sys.argv[1], sys.argv[2])
    if g.logged_in:
        print('Mail log in success')
except  gmail.AuthenticationError:  # could not log in
    print('log in fail')

list_mail(sys.argv[1])
g.logout
