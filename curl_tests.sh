#!/bin/bash
# curl_tests.sh - quick HTTP test script for your webserver
# Run from WSL or any bash shell with curl installed.

BASE_URL="http://127.0.0.1:8080"

echo "\n1) Root path /"
curl -i "$BASE_URL/"

echo "\n2) Explicit index /index.html"
curl -i "$BASE_URL/index.html"

echo "\n3) Static file test (update filename if needed)"
curl -i "$BASE_URL/somefile.txt"

echo "\n4) Autoindex directory /images/"
curl -i "$BASE_URL/images/"

echo "\n5) Autoindex file /images/<existing-file>"
echo "Replace <existing-file> with a real file name in assets/images"
# curl -i "$BASE_URL/images/<existing-file>"

echo "\n6) Redirect /old"
curl -i "$BASE_URL/old"

echo "\n7) Missing page /notfound"
curl -i "$BASE_URL/notfound"

echo "\n8) Method not allowed on root using DELETE"
curl -i -X DELETE "$BASE_URL/"

echo "\n9) Upload POST (if enabled at /upload)"
echo "Replace path/to/local.file with a real file path"
# curl -i -X POST -F 'file=@path/to/local.file' "$BASE_URL/upload"

echo "\nTest script complete. Update commented lines for local files as needed."
