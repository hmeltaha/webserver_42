#!/usr/bin/env python3
import os
import sys

method = os.environ.get('REQUEST_METHOD', 'GET')
query  = os.environ.get('QUERY_STRING', '')

print("Content-Type: text/html")
print("")
print("<html><body>")
print("<h1>CGI Works</h1>")
print("<p>Method: " + method + "</p>")
print("<p>Query: " + query + "</p>")
print("</body></html>")