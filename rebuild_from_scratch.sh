DATA_DIR="/usr/local/pgsql/data"
PG_BIN="/usr/local/pgsql/bin"
PG_USER="postgres"

./stop_server.sh

./configure
make clean
make -j 8
sudo make install

sudo rm -rf "$DATA_DIR"
sudo mkdir -p "$DATA_DIR"
sudo chown $PG_USER "$DATA_DIR"

su - $PG_USER -c "$PG_BIN/initdb -D $DATA_DIR"

./start_server.sh