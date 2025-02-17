FROM ubuntu:24.10

# Essentials
RUN \
  apt-get update && \
  DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    ca-certificates \
    software-properties-common \
    build-essential \
    cmake \
    python3 \
    python3-venv

# Install python packages in virtual environment
ENV PYTHONDONTWRITEBYTECODE=1
ENV PYTHON_VENV=/opt/venv
RUN \
  python3 -m venv $PYTHON_VENV && \
  $PYTHON_VENV/bin/pip install --no-compile --no-cache-dir --upgrade \
    pip \
    setuptools && \
  $PYTHON_VENV/bin/pip install --no-compile --no-cache-dir \
    matplotlib \
    seaborn \
    pandas \
    argparse

# Add virtual environment to PATH
ENV PATH="${PYTHON_VENV}/bin:${PATH}"

# Build project
WORKDIR /app
COPY . ./jcdp_src
RUN \
  BUILD_DIR=`mktemp -d -p .` && \
  cmake -B $BUILD_DIR -S jcdp_src -DCMAKE_INSTALL_PREFIX=/app && \
  cmake --build $BUILD_DIR && \
  cmake --install $BUILD_DIR && \
  rm -rf $BUILD_DIR

