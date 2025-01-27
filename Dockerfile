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
RUN chmod -R +x /home/pintos/utils
RUN ln -s $(which qemu-system-x86_64) /usr/local/bin/qemu
RUN ln -s /home/pintos/utils/pintos /usr/local/bin/pintos
RUN cp /home/pintos/utils/Pintos.pm /usr/local/bin

WORKDIR /home/pintos/threads
RUN make clean && make

WORKDIR /home
CMD ["/bin/bash"]