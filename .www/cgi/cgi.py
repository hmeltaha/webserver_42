#!/usr/bin/env python3

import time

# Simulate long CGI processing
time.sleep(21)

# CGI response
print("Status: 200 OK")
print("Content-Type: text/html")
print()

print("""
<!DOCTYPE html>
<html>
<head>
    <title>Slow CGI</title>
</head>
<body>
    <h1>CGI Finished After 15 Seconds</h1>
</body>
</html>
""")
