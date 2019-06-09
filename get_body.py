#-*- coding: utf-8 -*-
import gmail
import sys
import io

reload(sys)
sys.setdefaultencoding('utf-8')

def get_mail_body(mail_id) :
    all_box_mail = g.inbox().mail()
    for newmail in all_box_mail:
        newmail.fetch()
        if mail_id == newmail.message_id:
            body = str(unicode(newmail.body))
            print(body)
            break

g = gmail.login(sys.argv[1], sys.argv[2])
g.logged_in

get_mail_body(sys.argv[3])
g.logout
