[![CI](https://github.com/swift-nav/libswiftnav/actions/workflows/ci.yaml/badge.svg)](https://github.com/swift-nav/libswiftnav/actions/workflows/ci.yaml)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=swift-nav_libswiftnav&metric=alert_status)](https://sonarcloud.io/dashboard?id=swift-nav_libswiftnav)


libswiftnav
===========

Libswiftnav (LSN) is a platform independent library that implements GNSS utility functions for use by software-defined GNSS receivers or software requiring GNSS functionality. It is intended to be as portable as possible and is written in standards compliant C with no dependancies

LSN does not provide any functionality for communicating with Swift Navigation receivers.  See [libsbp](https://github.com/swift-nav/libsbp) to communicate with receivers using Swift Binary Protocol (SBP).

To checkout the library run the following commands in an appropriate directory
```
git clone git@github.com:swift-nav/libswiftnav.git
```
Which should checkout the source code

### Build
To build the library, run the following commands from the LSN root directory - LSN depends on the latest xcode for MacOSX and cmake
```
mkdir ./build
cd ./build
cmake ../
make -j4
```

# Build on Docker

The `libswiftnav` docker image is using a ECR-hosted base image `swift-build` that contains most swift build tools.

#### Get `swift-build` base image from ECR

To be able to pull this base image from ECR, you need to log into AWS (select the SSO-Build-User role) and then into 
ECR:

    aws-google-auth -S 115297745755 -I C02x4yyeb -p default -a -u <username>@swift-nav.com
    $(aws ecr get-login --no-include-email --region us-west-2 --registry-ids 571934480752)

#### Build the `swift-build` base image yourself (without ECR)

Alternatively to the AWS/ECR pull, you can build the base image yourself from the Dockerfile.base:
(replace \<tag\> with the one used by Dockerfile's FROM statement, e.g. 2018-12-20)

    docker build -t 571934480752.dkr.ecr.us-west-2.amazonaws.com/swift-build:<tag> modules/docker-recipes/swift-build

### Build the `libswiftnav` image and run container

Now you can build and run the libswiftnav image with

    docker-compose build libswiftnav
    docker-compose run libswiftnav 

#### Run a shell in docker

    make docker

- Starts a shell in a container, with the workspace mounted as /mnt/workspace.

#### Run build in docker

    make docker-build
    
#### Run clang-format

    make docker-lint
