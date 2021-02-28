# Base image is created by https://github.com/swift-nav/docker-recipes
FROM 571934480752.dkr.ecr.us-west-2.amazonaws.com/swift-build:2021-02-26

# Add anything that's specific to this repo's build environment here.
COPY tools/requirements.txt /tmp/libswiftnav_requirements.txt
RUN pip3 install -r /tmp/libswiftnav_requirements.txt
WORKDIR /mnt/workspace
