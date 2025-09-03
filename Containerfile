FROM fedora:latest AS socle
MAINTAINER sebastien.lefevre@abls-habitat.fr
RUN echo "Installing Git"
RUN /bin/dnf install git wget -y
RUN echo "Upgrading" 
RUN /bin/dnf upgrade -y

FROM socle
MAINTAINER sebastien.lefevre@abls-habitat.fr
EXPOSE 5559
WORKDIR /Watchdog
COPY ./ ./
RUN ./INSTALL.sh
RUN ./autogen.sh
RUN make install
RUN ldconfig
ENV ABLS_IN_A_CONTAINER 1
CMD /usr/local/bin/Watchdogd

