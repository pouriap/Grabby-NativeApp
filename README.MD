# Grabby Native Application

This is the native application for the Grabby addon.

If you are an ordinary Grabby user you do not need to do anything with this repository. This application comes included in the Grabby Toolkit and is automatically set up by the installer.


## What is a native application?

WebExtensions are much more limited in what they can do compared to legacy Firefox addons. They were intentionally limited for "security" reasons. 

Previously an addon could communicate with the operating system, call other programs, create files, etc. But now if an addon wants to communicate with another program, for example to start a download manager, it cannot directly do it. In order to do so it needs a "native application" to be installed on the host computer. And through that application it can communicate with the operating system and other programs.

This is the native application for Grabby.

It is mainly used to send commands to FlashGot and yt-dlp in order to perform downloads.


The application is written in C++ using Visual Studio 2012.