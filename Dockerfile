FROM ubuntu:18.04 AS builder

RUN apt-get update -qq && \
    apt-get install -y --no-install-recommends \
      software-properties-common && \
    add-apt-repository -y ppa:team-gcc-arm-embedded/ppa && \
    apt-get update -qq && \
    apt-get install -y --no-install-recommends \
      git make cmake python3 \
      gcc-arm-embedded && \
    apt-get autoremove -y && \
    apt-get clean -y && \
    rm -rf /var/lib/apt/lists/*

# Project sources volume should be mounted at /app
COPY . /opt/microbit-samples
WORKDIR /opt/microbit-samples

RUN python3 build.py

FROM scratch AS export-stage
COPY --from=builder /opt/microbit-samples/MICROBIT.bin .
COPY --from=builder /opt/microbit-samples/MICROBIT.hex .

ENTRYPOINT ["/bin/bash"]
