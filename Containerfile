FROM fedora:38
MAINTAINER sebastien.lefevre@abls-habitat.fr
EXPOSE 5559
RUN echo "Installing Git"
RUN /bin/dnf install git wget -y
RUN echo "Installing Fedora dependencies"
RUN dnf update -y
RUN dnf install -y libtool automake autoconf gcc gcc-c++ redhat-rpm-config
RUN dnf install -y glib2-devel openssl
RUN dnf install -y nut-devel mariadb-devel libuuid-devel
RUN dnf install -y popt-devel libsoup3-devel gtts
RUN dnf install -y json-glib-devel gammu-devel
RUN dnf install -y mpg123 sox libusb1-devel libgpiod-devel
RUN dnf install -y librsvg2-devel libstrophe-devel libphidget22-devel
RUN dnf install -y systemd-devel libjwt-devel
WORKDIR /Watchdog
COPY ./ ./
RUN ./autogen.sh
RUN make install
RUN echo "/usr/local/lib" > /etc/ld.so.conf.d/local.conf
RUN echo "/usr/local/lib64" >> /etc/ld.so.conf.d/local.conf
RUN ldconfig
ENV ABLS_IN_A_CONTAINER
CMD /usr/local/bin/Watchdogd
