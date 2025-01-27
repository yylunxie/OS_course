FROM ubuntu:16.04


RUN apt update && apt install -y --no-install-recommends \
    ca-certificates \
    build-essential \
    gdb \
    qemu-system-x86 \
    nasm \
    vim\
    perl \
    python3 \
    texinfo \
    unzip \ 
    wget && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /home
RUN wget https://github.com/Xueyi-Chen-David/pintos/archive/refs/heads/main.zip --no-check-certificate && \
    unzip main.zip && \
    mv pintos-main pintos && \
    rm main.zip

WORKDIR /home/pintos/utils
# RUN make
# ENV PATH="/pintos/src/utils:$PATH"

# WORKDIR /home/pintos/src/threads
# RUN make clean && make

# WORKDIR /home
CMD ["/bin/bash"]