#
# MIT License
#
# Copyright (c) 2024 Maarten Hoeben
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
FROM ubuntu:22.04

# Set the user and group IDs of the host user.
ARG USER_ID
ARG GROUP_ID

# Create a new user inside the container with the same IDs as the host user.
RUN addgroup --gid $GROUP_ID bob \
    && adduser --disabled-password --gecos '' --uid $USER_ID --gid $GROUP_ID bob

# Install dependencies.
RUN apt-get update && \
    apt-get install -y clang uuid-dev python3-pip pkg-config ninja-build
RUN pip3 install meson

# Cleanup.
RUN apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory inside the container.
WORKDIR /workspace

# Switch to the newly created user.
USER bob

# Create a volume for the project's root directory.
VOLUME /workspace

# Set environment variables.
ENV CC=clang
ENV CXX=clang++

CMD meson setup --wipe build -Dhlib_build_tests=true \
 && meson test -C build
