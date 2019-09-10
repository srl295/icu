# 
# © 2016 and later: Unicode, Inc. and others.
# License & terms of use: http://www.unicode.org/copyright.html#License#
#
# You can use this image to build docs as following:
#
# $ cd …/src/icu/icu4j/
# $ (cd tools/build/docker/openjdk7/ && docker build . -t icu4j-build-7:latest)
# $ docker run --rm -it -v $(pwd):/src -w /src icu4j-build-7:latest ant docs
#
# It’s probably useful for other things.

FROM openjdk:7
# install ant
RUN apt-get update
RUN apt-get install -y ant unzip
# install into /opt/jcite
RUN mkdir -p /opt && cd /opt &&  curl https://web.archive.org/web/20160710183051/http://jcite.googlecode.com/files/jcite-1.13.0-bin.zip > jcite.zip && unzip jcite.zip && ln -sv jcite-* jcite
ENV JCITE_DIR=/opt/jcite
