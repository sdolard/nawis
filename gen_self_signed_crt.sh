#! /bin/bash
out_dir=certificate
[ ! -d $out_dir ] && mkdir $out_dir

# Creating a private key
openssl genrsa -des3 -out "$out_dir/$USER.key" 1024 
chmod 400 "$out_dir/$USER.key"

# This certificate request
# Format is already pem
openssl req -new -days 1001 -key "$out_dir/$USER.key" -out "$out_dir/$USER.csr"

# Create self-signed certificate
# Format is already pem
openssl x509 -req -days 365 -in "$out_dir/$USER.csr" -signkey "$out_dir/$USER.key" -out "$out_dir/$USER.crt"

echo "Done: everything build in $out_dir dir."
