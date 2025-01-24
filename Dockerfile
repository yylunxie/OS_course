FROM ubuntu:16.04


RUN apt update && apt install -y --no-install-recommends \
    ca-certificates \
    build-essential \
    gdb \
    qemu-system-x86 \
    nasm \
    perl \
    python3 \
    texinfo \
    wget && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /home
RUN wget https://www.stanford.edu/class/cs140/projects/pintos/pintos.tar.gz --no-check-certificate && \
    tar -xzf pintos.tar.gz && \
    rm pintos.tar.gz

WORKDIR /home/pintos/src/utils
# RUN make
# ENV PATH="/pintos/src/utils:$PATH"

# WORKDIR /home/pintos/src/threads
# RUN make clean && make

# WORKDIR /home
CMD ["/bin/bash"]