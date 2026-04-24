#!/usr/bin/python3
#coding:utf-8
import sys,os
import urllib.parse

print("Content-type:text/html\n")

print('<html>')
print('<head>')
print('<title>CGI Test</title>')
print('</head>')
print('<body>')
print('<h2>CGI Script Executed Successfully!</h2>')

# 显示环境变量
print('<h3>Environment Variables:</h3>')
print('<ul>')
for key in sorted(os.environ.keys()):
    print(f'<li>{key} = {os.environ[key]}</li>')
print('</ul>')

# 处理POST数据
content_length = os.environ.get('CONTENT_LENGTH')
if content_length:
    try:
        length = int(content_length)
        postdata = sys.stdin.read(length)
        print('<h3>POST Data:</h3>')
        print('<pre>' + postdata + '</pre>')
    except:
        print('<p>Error reading POST data</p>')

print('</body>')
print('</html>')
#     print '</html>'
    
# else:
#     print "Content-type:text/html\n"
#     print 'no found'

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import urllib.parse
import html

length = os.getenv('CONTENT_LENGTH')

print("Content-Type: text/html")
print()

if length:
    postdata = sys.stdin.read(int(length))
    print("<html>")
    print("<body>")
    print("<h2>Your POST data:</h2>")
    print("<ul>")
    
    for data in postdata.split('&'):
        decoded = urllib.parse.unquote_plus(data)
        safe = html.escape(decoded)
        print(f"<li>{safe}</li>")
    
    print("</ul>")
    print("</body>")
    print("</html>")
else:
    print("No POST data found")

