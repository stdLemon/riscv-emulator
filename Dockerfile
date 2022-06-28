FROM ubuntu
WORKDIR /emulator
ARG riscv_deps="autoconf automake autotools-dev curl python3 libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev"
RUN apt update && apt -y install g++ cmake git
RUN apt -y install ${riscv_deps} && git clone https://github.com/riscv-collab/riscv-gnu-toolchain
RUN cd riscv-gnu-toolchain && ./configure --prefix=/riscv --with-arch=rv32i && make -j$(nproc)
COPY ./src src
COPY ./CMakeLists.txt .
RUN mkdir build && cmake -B build && cmake --build build/

FROM ubuntu
WORKDIR /emulator
COPY --from=0 /emulator/build/riscv-emulator .
COPY --from=0 /riscv riscv-gnu-toolchain
RUN apt update && apt -y install libmpc-dev libmpfr-dev
COPY ./example_programs example_programs

