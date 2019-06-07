import gmail
import sys
import io
f = open('login.txt', mode='wt')
try:
    g = gmail.login(sys.argv[1], sys.argv[2])
    if g.logged_in:
        g.logout
        f.write('1')
        f.close()
except  gmail.AuthenticationError:  # could not log in
    f.write('0')
    f.close()
