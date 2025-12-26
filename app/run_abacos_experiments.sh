#!/usr/bin/env bash

set -euo pipefail

# ========= Experiment Parameters =========
PERIOD_MS=100
DURATION_SECONDS=600     # 10 minutes
PAYLOAD_SIZES=(64 256 1024 4096 16384 65536)

VSOMEIP_PUB=/app/abacos/app/abacos-vsomeip/bin/abacos-vsomeip-publisher.o
VSOMEIP_SUB=/app/abacos/app/abacos-vsomeip/bin/abacos-vsomeip-subscriber.o

DDS_PUB=./abacos-dds/bin/abacos-dds-publisher.o
DDS_SUB=./abacos-dds/bin/abacos-dds-subscriber.o

LOGDIR=./experiments_logs
mkdir -p "$LOGDIR"

# ========= Helper to run one experiment =========
run_pair () {
    local name=$1
    local pub=$2
    local sub=$3
    local payload=$4

    echo "--------------------------------------------------"
    echo " Running $name | payload=${payload} bytes | period=${PERIOD_MS} ms"
    echo " Duration: ${DURATION_SECONDS}s"
    echo "--------------------------------------------------"

    local prefix="${LOGDIR}/${name}_${payload}B"

    # Start subscriber first
    "${sub}" "${payload}" > "${prefix}_sub.log" 2>&1 &
    SUB_PID=$!

    sleep 1   # ensure subscriber is ready

    # Run publisher with timeout
    timeout "${DURATION_SECONDS}" \
        "${pub}" "${PERIOD_MS}" "${payload}" > "${prefix}_pub.log" 2>&1 || true

    # Stop subscriber after publisher stops
    kill "${SUB_PID}" >/dev/null 2>&1 || true
    wait "${SUB_PID}" 2>/dev/null || true

    echo "Completed: ${name} payload=${payload}"
    echo
}

# ========= Experiment Loop =========
for payload in "${PAYLOAD_SIZES[@]}"; do

    run_pair "VSOMEIP" "$VSOMEIP_PUB" "$VSOMEIP_SUB" "$payload"

    #run_pair "DDS" "$DDS_PUB" "$DDS_SUB" "$payload"

done

echo "All experiments completed. Logs in: $LOGDIR"
