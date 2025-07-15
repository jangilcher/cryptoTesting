#
# Ubuntu Dockerfile
#
# https://github.com/dockerfile/ubuntu
#
#
# To build the image:
# $ docker build -t u18 .
#
# To run the image in a temporary container
# $ docker run -it --rm u18
#
# To dump the image to temporary
# $ docker save u18 > u18.tar
#
# To remove the image
# $ docker rmi u18
#
#

# Pull base image.
FROM ubuntu:18.04
ENV DEBIAN_FRONTEND="noninteractive"

# Install essentials
RUN \
    apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends build-essential git wget psmisc python3-psutil sudo

# Install clang-11 and llvm-11 for compiling aflpp on ubuntu18
RUN \
    apt-get install -y lsb-release wget software-properties-common gnupg && \
    (wget --no-check-certificate -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - ) && \
    add-apt-repository 'deb http://apt.llvm.org/bionic/   llvm-toolchain-bionic-11  main' && \
    apt-get update && \
    apt-get install -y --no-install-recommends llvm-11 lldb-11 llvm-11-dev libllvm11 llvm-11-runtime clang-11 && \
    update-alternatives --install /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-11 11 && \
    update-alternatives --install /usr/bin/clang clang /usr/bin/clang-11 11

# install software needed by the supercop tests
RUN \
    apt-get install -y --no-install-recommends libssl-dev libgmp-dev libcrypto++-dev

# Install optional
RUN apt-get install -y --no-install-recommends byobu nano bash-completion less
RUN ln -s /usr/bin/python3 /usr/bin/python

# Install micro editor
RUN apt-get install -y --no-install-recommends curl
RUN mkdir -p /root/bin
RUN bash -c "curl https://getmic.ro | bash"
RUN mv /micro /root/bin
ENV PATH="/root/bin:${PATH}"

# RUN echo core >/proc/sys/kernel/core_pattern
#   sed -i 's/# \(.*multiverse$\)/\1/g' /etc/apt/sources.list && \
#   apt-get update && \
#   apt-get -y upgrade && \
#   apt-get install -y build-essential && \
#   apt-get install -y byobu curl git htop man unzip vim wget && \
#   apt-get install -y software-properties-common && \
#   rm -rf /var/lib/apt/lists/*

ADD docker/ubuntu/root/.bashrc /root/.bashrc
ADD docker/ubuntu/root/.gitconfig /root/.gitconfig
ADD docker/ubuntu/root/.scripts /root/.scripts

# Add makefile with initial dependencies.
RUN mkdir -p /fuzzing
COPY Makefile /fuzzing/Makefile

# Define working directory.
WORKDIR /fuzzing
ENV HOME /root

# Install dependences
RUN make all_deps

# cop over testing files
COPY fuzz_liboqs.py /fuzzing/fuzz_liboqs.py
COPY fuzz_liboqs_baseline.py /fuzzing/fuzz_liboqs_baseline.py
COPY report.py /fuzzing/report.py
COPY report_baseline.py /fuzzing/report_baseline.py
RUN mkdir -p /fuzzing/tech/paper_fuzzing/vanilla
COPY tech/paper_fuzzing/vanilla/. /fuzzing/tech/paper_fuzzing/vanilla
RUN mkdir -p /fuzzing/tech/paper_fuzzing/liboqs
COPY tech/paper_fuzzing/liboqs/. /fuzzing/tech/paper_fuzzing/liboqs
RUN mkdir -p /fuzzing/tech/paper_fuzzing/utilities
COPY tech/paper_fuzzing/utilities/. /fuzzing/tech/paper_fuzzing/utilities
RUN mkdir -p /fuzzing/tech/paper_fuzzing/supercop/crypto_hash
COPY tech/paper_fuzzing/supercop/crypto_hash/. /fuzzing/tech/paper_fuzzing/supercop/crypto_hash
COPY supercop_report.py /fuzzing/supercop_report.py
COPY supercop_report_baseline.py /fuzzing/supercop_report_baseline.py

RUN make aflpp
# RUN make cur_liboqs
# RUN make old_liboqs
# RUN make mid_liboqs
# RUN make supercop

# spreadsheet tools for report
RUN apt-get install --no-install-recommends -y python3-openpyxl python3-pip
VOLUME [ "/fuzzing/reports" ]

# If working on the folder on disk rather than copying it:
# RUN bash -c 'echo "cd /fuzzing && make cur_liboqs" >> /root/.bashrc'
# RUN bash -c 'echo "cd /fuzzing && make mid_liboqs" > /root/.bashrc'
# RUN bash -c 'echo "cd /fuzzing && make old_liboqs" > /root/.bashrc'
# RUN bash -c 'echo "cd /fuzzing && make aflpp" >> /root/.bashrc'

# Define default command.
CMD ["bash"]
