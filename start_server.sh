echo "/opt/intel/sgxsdk/lib64" | sudo tee /etc/ld.so.conf.d/sgx.conf
sudo ldconfig
su - postgres -c "/usr/local/pgsql/bin/pg_ctl -D /usr/local/pgsql/data -l /usr/local/pgsql/data/logfile start"
/usr/local/pgsql/bin/psql -d postgres -U postgres