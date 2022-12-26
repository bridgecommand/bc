# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/davide/bc/DEC22/bc/src/libs/Irrlicht/irrlicht-svn/source/Irrlicht"
  "/home/davide/bc/DEC22/bc/src/libs/Irrlicht/irrlicht-svn/source/Irrlicht"
  "/home/davide/bc/DEC22/bc/bin/libs/Irrlicht/bc-irrlicht-internal-prefix"
  "/home/davide/bc/DEC22/bc/bin/libs/Irrlicht/bc-irrlicht-internal-prefix/tmp"
  "/home/davide/bc/DEC22/bc/bin/libs/Irrlicht/bc-irrlicht-internal-prefix/src/bc-irrlicht-internal-stamp"
  "/home/davide/bc/DEC22/bc/bin/libs/Irrlicht/bc-irrlicht-internal-prefix/src"
  "/home/davide/bc/DEC22/bc/bin/libs/Irrlicht/bc-irrlicht-internal-prefix/src/bc-irrlicht-internal-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/davide/bc/DEC22/bc/bin/libs/Irrlicht/bc-irrlicht-internal-prefix/src/bc-irrlicht-internal-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/davide/bc/DEC22/bc/bin/libs/Irrlicht/bc-irrlicht-internal-prefix/src/bc-irrlicht-internal-stamp${cfgdir}") # cfgdir has leading slash
endif()
