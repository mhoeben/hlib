# Description #

# Prerequisites #

* Docker (for Ubuntu refer to https://docs.docker.com/engine/install/ubuntu/)

# Build #

## Setup ##

```
docker build -t hlib --build-arg USER_ID=$(id -u) --build-arg GROUP_ID=$(id -g) .
```

## Build ##

```
docker run -it -v "$(pwd)":/workspace hlib
```

