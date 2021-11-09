# Tasks
## Include dependencies for GameNetworkingSockets into repo
protobuf and openssl were installed as deb package:
`sudo apt install libssl-dev libprotobuf-dev protobuf-compiler`
include them into repo and compile into setup.py

## Add switch for setup.py to compile libs in Release and Debug mode
currently everything is compiled in debug mode
