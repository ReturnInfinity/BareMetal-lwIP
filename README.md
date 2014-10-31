# lwIP on BareMetal

## A lightweight TCP/IP stack running on an ultra-lightweight kernel

This is a quick and simple proof of concept web server running on top of the lightweight TCP/IP stack [lwIP](http://savannah.nongnu.org/projects/lwip/), which in turn is running on top of [BareMetal](http://www.returninfinity.com/baremetal.html). The BareMetal binary is a mere 16,384 bytes and the web server application is appoximately 170,000 bytes.

BareMetal is a new [Exokernel](http://en.wikipedia.org/wiki/Exokernel) for 64-bit x86-based systems. The kernel, as well as its various device drivers, are written entirely in Assembly for optimal speed, simplicity, and manageability of the code. BareMetal provides raw access to the underlying hardware in order to allow for maximum performance. Applications can be specifically tailored to focus on where performance is needed the most.

More information about BareMetal can be found on the BareMetal homepage at [Return Infinity](http://www.returninfinity.com/baremetal.html). The source code is available for viewing on [GitHub](https://github.com/ReturnInfinity/BareMetal-OS).

lwIP is an open source implementation of the TCP/IP protocol suite that was originally written by [Adam Dunkels](http://www.sics.se/~adam/) of the Swedish Institute of Computer Science but now is being actively improved upon by a team of developers throughout the world.

More information about lwIP can be found at the [lwIP homepage](http://savannah.nongnu.org/projects/lwip/).

## Building the web server

  git clone https://github.com/ReturnInfinity/BareMetal-lwIP.git
  cd BareMetal-lwIP
  make

## Changing the web content

The web page content is in httpd_raw/fs/ so make all changes there.

  cd httpd_raw
  perl makefsdata/makefsdata -s
  cd ..
  make
