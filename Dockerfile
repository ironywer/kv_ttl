# ---------- build stage ----------
FROM ubuntu:22.04 AS builder

ARG DEBIAN_FRONTEND=noninteractive
ARG CMAKE_BUILD_TYPE=Release

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git ca-certificates lcov gcovr && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY CMakeLists.txt ./
COPY include ./include
COPY src ./src
COPY tests ./tests

RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="--coverage" \
    -DCMAKE_EXE_LINKER_FLAGS="--coverage" \
    && cmake --build build -j \
    && ctest --test-dir build --output-on-failure \
    && cd build \
    && lcov --capture --directory . --output-file coverage.info \
    && lcov --remove coverage.info '/usr/*' '*/_deps/*' --output-file coverage.info \
    && genhtml coverage.info --output-directory coverage-html \
    && gcovr -r .. --exclude '.*_deps/.*'  # текстовый отчёт в логе (опционально)
# ---------- runtime stage ----------
FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends libstdc++6 && \
    rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/kvstorage_example /usr/local/bin/kvstorage_example
COPY --from=builder /app/build/kvstorage_tests   /usr/local/bin/kvstorage_tests
COPY --from=builder /app/build/coverage-html /opt/coverage

CMD ["kvstorage_example"]
