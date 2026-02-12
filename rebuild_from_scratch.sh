DATA_DIR="/usr/local/pgsql/data"
PG_BIN="/usr/local/pgsql/bin"
PG_USER="postgres"

clear

cd src/tee
make clean -f enclave.makefile
make -f enclave.makefile
cd ../../

./stop_server.sh

# ./configure
make clean
make -j 8
sudo make install

sudo cp src/tee/enclave.signed.so /usr/local/pgsql/lib/
sudo chown postgres:postgres /usr/local/pgsql/lib/enclave.signed.so
sudo chmod 755 /usr/local/pgsql/lib/enclave.signed.so

sudo rm -rf "$DATA_DIR"
sudo mkdir -p "$DATA_DIR"
sudo chown $PG_USER "$DATA_DIR"

su - $PG_USER -c "$PG_BIN/initdb -D $DATA_DIR"

./start_server.sh