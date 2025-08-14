su - postgres -c "/usr/local/pgsql/bin/pg_ctl -D /usr/local/pgsql/data stop"
make
sudo make install
su - postgres -c "/usr/local/pgsql/bin/pg_ctl -D /usr/local/pgsql/data -l /usr/local/pgsql/data/logfile start"
/usr/local/pgsql/bin/psql -d postgres -U postgres