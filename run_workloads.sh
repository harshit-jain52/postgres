#!/bin/bash

for n in 10 50 100 200 500 1000; do
    python3 generate_workload.py 30 $n
    su - postgres -c "./abac_workload.sh"
done
