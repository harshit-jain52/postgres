/usr/local/pgsql/bin/pg_ctl -D /usr/local/pgsql/data -l /usr/local/pgsql/data/logfile start
/usr/local/pgsql/bin/dropdb testdb
/usr/local/pgsql/bin/createdb testdb
/usr/local/pgsql/bin/psql -d testdb -f generated_workload.sql
# /usr/local/pgsql/bin/psql -d testdb -f workload_only_env.sql