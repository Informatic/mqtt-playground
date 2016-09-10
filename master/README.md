```
openssl req -nodes -new -x509 -days 3650 -extensions v3_ca -keyout ca.key -out ca.crt
openssl genrsa -out server.key 2048
openssl req -out server.csr -key server.key -new
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 3650

# Extract SHA-1
openssl x509 -in server.crt -sha1 -noout -fingerprint | sed 's/.*=/0x/g; s/:/, 0x/g'
```
