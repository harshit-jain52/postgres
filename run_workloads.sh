#!/bin/bash

for n in 10 50 100 200 500; do
    python3 generate_workload_resources.py $n 20 500
    su - postgres -c "./abac_workload.sh"
done
