FROM ubuntu:22.04 as builder-linux

RUN apt-get update && apt-get install -y \
    build-essential \
    libncurses5-dev \
    git

WORKDIR /build
COPY . .
RUN make clean && make

FROM ubuntu:22.04 as final
RUN apt-get update && apt-get install -y libncurses5
COPY --from=builder-linux /build/bitcamp /usr/local/bin/bitcamp
ENTRYPOINT ["bitcamp"]
